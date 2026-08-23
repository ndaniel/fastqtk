/* The MIT License

   Copyright (c) 2010-2026 Daniel Nicorici <daniel.nicorici@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

use std::env;
use std::ffi::c_void;
use std::fs::File;
use std::io::{self, BufWriter, Read, Write};
use std::process::ExitCode;

const BUFFER_SIZE: usize = 8 * 1024 * 1024;
const MAX_LEN: usize = 32 * 1024;
const RECORD_SLACK: usize = MAX_LEN * 2 + 1024;
const ALPHABET: &[u8] = b"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

type Result<T> = std::result::Result<T, String>;

extern "C" {
    fn memchr(bytes: *const c_void, byte: i32, len: usize) -> *mut c_void;
}

#[inline]
fn find_newline(bytes: &[u8]) -> Option<usize> {
    if bytes.is_empty() {
        return None;
    }
    // SAFETY: `bytes.as_ptr()` is valid for `bytes.len()` readable bytes.
    let found = unsafe { memchr(bytes.as_ptr().cast(), b'\n' as i32, bytes.len()) };
    if found.is_null() {
        None
    } else {
        Some((found as usize) - (bytes.as_ptr() as usize))
    }
}

#[derive(Default)]
struct Record {
    id: Vec<u8>,
    seq: Vec<u8>,
    plus: Vec<u8>,
    qual: Vec<u8>,
}

impl Record {
    fn write_original<W: Write + ?Sized>(&self, out: &mut W) -> io::Result<()> {
        write_line(out, &self.id)?;
        write_line(out, &self.seq)?;
        write_line(out, &self.plus)?;
        write_line(out, &self.qual)
    }

}

fn write_line<W: Write + ?Sized>(out: &mut W, line: &[u8]) -> io::Result<()> {
    out.write_all(line)?;
    out.write_all(b"\n")
}

struct FastqReader {
    inner: Box<dyn Read>,
    buffer: Vec<u8>,
    start: usize,
    end: usize,
    eof: bool,
}

#[derive(Clone, Copy)]
struct RecordView<'a> {
    raw: &'a [u8],
    id: &'a [u8],
    seq: &'a [u8],
    plus: &'a [u8],
    qual: &'a [u8],
    terminated: bool,
}

impl FastqReader {
    fn open(path: &str) -> Result<Self> {
        Ok(Self::new(raw_input(path)?))
    }

    fn new(inner: Box<dyn Read>) -> Self {
        Self::with_capacity(inner, BUFFER_SIZE)
    }

    fn with_capacity(inner: Box<dyn Read>, capacity: usize) -> Self {
        Self {
            inner,
            buffer: vec![0; capacity],
            start: 0,
            end: 0,
            eof: false,
        }
    }

    fn next_view(&mut self) -> io::Result<Option<RecordView<'_>>> {
        loop {
            let mut newlines = [0_usize; 4];
            let mut found = 0;
            let mut scan = self.start;
            while found < 4 {
                let Some(offset) = find_newline(&self.buffer[scan..self.end]) else {
                    break;
                };
                scan += offset;
                newlines[found] = scan;
                found += 1;
                scan += 1;
            }

            if found == 4 {
                let record_start = self.start;
                let record_end = newlines[3] + 1;
                self.start = record_end;
                return Ok(Some(RecordView {
                    raw: &self.buffer[record_start..record_end],
                    id: &self.buffer[record_start..newlines[0]],
                    seq: &self.buffer[newlines[0] + 1..newlines[1]],
                    plus: &self.buffer[newlines[1] + 1..newlines[2]],
                    qual: &self.buffer[newlines[2] + 1..newlines[3]],
                    terminated: true,
                }));
            }

            if self.eof {
                // The C implementation supplies a final newline when the last
                // FASTQ quality line does not have one.
                if found == 3 && scan < self.end {
                    let record_start = self.start;
                    let record_end = self.end;
                    self.start = self.end;
                    return Ok(Some(RecordView {
                        raw: &self.buffer[record_start..record_end],
                        id: &self.buffer[record_start..newlines[0]],
                        seq: &self.buffer[newlines[0] + 1..newlines[1]],
                        plus: &self.buffer[newlines[1] + 1..newlines[2]],
                        qual: &self.buffer[newlines[2] + 1..record_end],
                        terminated: false,
                    }));
                }
                return Ok(None);
            }

            self.compact_or_grow();
            let bytes = self.inner.read(&mut self.buffer[self.end..])?;
            if bytes == 0 {
                self.eof = true;
            } else {
                self.end += bytes;
            }
        }
    }

    fn next_record(&mut self, rec: &mut Record) -> io::Result<bool> {
        let Some(view) = self.next_view()? else {
            return Ok(false);
        };
        copy_field(&mut rec.id, view.id);
        copy_field(&mut rec.seq, view.seq);
        copy_field(&mut rec.plus, view.plus);
        copy_field(&mut rec.qual, view.qual);
        Ok(true)
    }

    fn next_line(&mut self, line: &mut Vec<u8>) -> io::Result<bool> {
        loop {
            if let Some(offset) = find_newline(&self.buffer[self.start..self.end]) {
                let newline = self.start + offset;
                copy_field(line, &self.buffer[self.start..newline]);
                self.start = newline + 1;
                return Ok(true);
            }
            if self.eof {
                if self.start == self.end {
                    return Ok(false);
                }
                copy_field(line, &self.buffer[self.start..self.end]);
                self.start = self.end;
                return Ok(true);
            }
            self.compact_or_grow();
            let bytes = self.inner.read(&mut self.buffer[self.end..])?;
            if bytes == 0 {
                self.eof = true;
            } else {
                self.end += bytes;
            }
        }
    }

    fn compact_or_grow(&mut self) {
        if self.start != 0 {
            self.buffer.copy_within(self.start..self.end, 0);
            self.end -= self.start;
            self.start = 0;
        }
        if self.end == self.buffer.len() {
            self.buffer.resize(self.buffer.len() * 2, 0);
        }
    }
}

fn copy_field(dst: &mut Vec<u8>, src: &[u8]) {
    dst.clear();
    dst.extend_from_slice(src);
}

fn raw_input(path: &str) -> Result<Box<dyn Read>> {
    if path == "-" {
        Ok(Box::new(io::stdin()))
    } else {
        File::open(path)
            .map(|f| Box::new(f) as Box<dyn Read>)
            .map_err(|_| format!("ERROR: Failed to open the file '{path}'!"))
    }
}

enum Output {
    File(BufWriter<File>),
    Stdout(BufWriter<io::Stdout>),
}

impl Write for Output {
    #[inline]
    fn write(&mut self, bytes: &[u8]) -> io::Result<usize> {
        match self {
            Self::File(writer) => writer.write(bytes),
            Self::Stdout(writer) => writer.write(bytes),
        }
    }

    #[inline]
    fn flush(&mut self) -> io::Result<()> {
        match self {
            Self::File(writer) => writer.flush(),
            Self::Stdout(writer) => writer.flush(),
        }
    }
}

fn output(path: &str) -> Result<Output> {
    if path == "-" {
        Ok(Output::Stdout(BufWriter::with_capacity(
            BUFFER_SIZE,
            io::stdout(),
        )))
    } else {
        File::create(path)
            .map(|f| {
                Output::File(BufWriter::with_capacity(BUFFER_SIZE, f))
            })
            .map_err(|_| format!("ERROR: Failed to open the file '{path}'!"))
    }
}

fn parse_usize(value: &str) -> Result<usize> {
    value
        .parse::<usize>()
        .map_err(|_| format!("ERROR: Expected a non-negative integer, got '{value}'."))
}

fn for_each_view(
    in_path: &str,
    out_path: &str,
    mut action: impl FnMut(RecordView<'_>, &mut Vec<u8>) -> Result<()>,
) -> Result<()> {
    let mut reader = FastqReader::open(in_path)?;
    let mut writer = output(out_path)?;
    let mut block = Vec::with_capacity(BUFFER_SIZE + RECORD_SLACK);
    while let Some(rec) = reader.next_view().map_err(read_error)? {
        prepare_block(&mut block, rec.raw.len() + 64, &mut writer)?;
        action(rec, &mut block)?;
        if block.len() >= BUFFER_SIZE {
            writer.write_all(&block).map_err(write_error)?;
            block.clear();
        }
    }
    writer.write_all(&block).map_err(write_error)?;
    writer.flush().map_err(write_error)
}

#[inline]
fn prepare_block<W: Write>(block: &mut Vec<u8>, needed: usize, out: &mut W) -> Result<()> {
    if block.capacity() - block.len() < needed {
        out.write_all(block).map_err(write_error)?;
        block.clear();
        if block.capacity() < needed {
            block.reserve(needed);
        }
    }
    Ok(())
}

#[inline]
fn append_bytes(out: &mut Vec<u8>, bytes: &[u8]) {
    debug_assert!(out.capacity() - out.len() >= bytes.len());
    let old_len = out.len();
    // SAFETY: `prepare_block` reserves space for the complete output record,
    // the source does not overlap the output block, and length is updated only
    // after all bytes have been initialized.
    unsafe {
        std::ptr::copy_nonoverlapping(
            bytes.as_ptr(),
            out.as_mut_ptr().add(old_len),
            bytes.len(),
        );
        out.set_len(old_len + bytes.len());
    }
}

#[inline]
fn append_line(out: &mut Vec<u8>, line: &[u8]) {
    append_bytes(out, line);
    append_bytes(out, b"\n");
}

#[inline]
fn append_original(out: &mut Vec<u8>, rec: RecordView<'_>) {
    append_bytes(out, rec.raw);
    if !rec.terminated {
        append_bytes(out, b"\n");
    }
}

#[inline]
fn append_normalized_parts(out: &mut Vec<u8>, id: &[u8], seq: &[u8], qual: &[u8]) {
    append_line(out, id);
    append_line(out, seq);
    append_bytes(out, b"+\n");
    append_line(out, qual);
}

fn read_error(_: io::Error) -> String {
    "ERROR: Failed reading the input file.".into()
}

fn write_error(e: io::Error) -> String {
    format!("ERROR: Failed writing output: {e}")
}

fn usage() -> &'static str {
    "Usage:   fastqtk <command> <arguments>\nVersion: 0.29 (Rust)\n\n\
Commands:\n  interleave deinterleave count lengths count-lengths\n  tab-4 tab-8 detab drop-short NtoA trim-5 trim-3\n  retain-5 retain-3 trim-id trim-poly compress-id drop-se\n  fq2fa fa2fq rev-com"
}

fn command(args: &[String]) -> Result<()> {
    let cmd = args
        .get(1)
        .map(String::as_str)
        .ok_or_else(|| String::from(usage()))?;
    match cmd {
        "deinterleave" if args.len() == 5 => deinterleave(&args[2], &args[3], &args[4]),
        "interleave" if args.len() == 5 => interleave(&args[2], &args[3], &args[4]),
        "count" if args.len() == 4 => count(&args[2], &args[3]),
        "lengths" if args.len() == 4 => lengths(&args[2], None, &args[3]),
        "count-lengths" if args.len() == 5 => lengths(&args[2], Some(&args[3]), &args[4]),
        "tab-4" if args.len() == 4 => tab(&args[2], &args[3], 4),
        "tab-8" if args.len() == 4 => tab(&args[2], &args[3], 8),
        "detab" if args.len() == 4 => detab(&args[2], &args[3]),
        "drop-short" if args.len() == 5 => drop_short(parse_usize(&args[2])?, &args[3], &args[4]),
        "NtoA" if args.len() == 4 => ntoa(&args[2], &args[3]),
        "trim-5" if args.len() == 5 => trim(parse_usize(&args[2])?, true, &args[3], &args[4]),
        "trim-3" if args.len() == 5 => trim(parse_usize(&args[2])?, false, &args[3], &args[4]),
        "retain-5" if args.len() == 5 => retain(parse_usize(&args[2])?, true, &args[3], &args[4]),
        "retain-3" if args.len() == 5 => retain(parse_usize(&args[2])?, false, &args[3], &args[4]),
        "trim-id" if args.len() == 4 => trim_id(&args[2], &args[3]),
        "trim-poly" if args.len() == 6 => trim_poly(&args[2], parse_usize(&args[3])?, &args[4], &args[5]),
        "compress-id" if (4..=6).contains(&args.len()) => compress_id(args),
        "drop-se" if args.len() == 4 => drop_se(&args[2], &args[3]),
        "fq2fa" if args.len() == 4 => fq2fa(&args[2], &args[3]),
        "fa2fq" if args.len() == 4 => fa2fq(&args[2], &args[3]),
        "rev-com" if args.len() == 4 => rev_com(&args[2], &args[3]),
        _ => Err(usage().into()),
    }
}

fn deinterleave(src: &str, first: &str, second: &str) -> Result<()> {
    let mut reader = FastqReader::open(src)?;
    let mut out1 = output(first)?;
    let mut out2 = output(second)?;
    let mut block1 = Vec::with_capacity(BUFFER_SIZE + RECORD_SLACK);
    let mut block2 = Vec::with_capacity(BUFFER_SIZE + RECORD_SLACK);
    let mut odd = true;
    while let Some(rec) = reader.next_view().map_err(read_error)? {
        let (block, out) = if odd { (&mut block1, &mut out1) } else { (&mut block2, &mut out2) };
        prepare_block(block, rec.raw.len() + 1, out)?;
        append_original(block, rec);
        if block.len() >= BUFFER_SIZE {
            out.write_all(block).map_err(write_error)?;
            block.clear();
        }
        odd = !odd;
    }
    out1.write_all(&block1).map_err(write_error)?;
    out2.write_all(&block2).map_err(write_error)?;
    out1.flush().map_err(write_error)?;
    out2.flush().map_err(write_error)
}

fn interleave(first: &str, second: &str, dst: &str) -> Result<()> {
    let mut in1 = FastqReader::open(first)?;
    let mut in2 = FastqReader::open(second)?;
    let mut out = output(dst)?;
    let mut block = Vec::with_capacity(BUFFER_SIZE + RECORD_SLACK);
    loop {
        let one = in1.next_view().map_err(read_error)?;
        let two = in2.next_view().map_err(read_error)?;
        if let Some(rec) = one {
            prepare_block(&mut block, rec.raw.len() + 1, &mut out)?;
            append_original(&mut block, rec);
        }
        if let Some(rec) = two {
            prepare_block(&mut block, rec.raw.len() + 1, &mut out)?;
            append_original(&mut block, rec);
        }
        if block.len() >= BUFFER_SIZE {
            out.write_all(&block).map_err(write_error)?;
            block.clear();
        }
        if one.is_none() && two.is_none() { break; }
    }
    out.write_all(&block).map_err(write_error)?;
    out.flush().map_err(write_error)
}

fn count(src: &str, dst: &str) -> Result<()> {
    let mut reader = raw_input(src)?;
    let mut buf = vec![0_u8; BUFFER_SIZE];
    let (mut lines, mut bytes, mut last) = (0_u64, 0_u64, b'\n');
    loop {
        let n = reader.read(&mut buf).map_err(read_error)?;
        if n == 0 { break; }
        bytes += n as u64;
        lines += buf[..n].iter().filter(|&&c| c == b'\n').count() as u64;
        last = buf[n - 1];
    }
    if bytes > 0 && last != b'\n' { lines += 1; }
    let reads = if lines > 2 { (lines + 3) / 4 } else { 0 };
    let mut out = output(dst)?;
    writeln!(out, "{reads}").map_err(write_error)?;
    out.flush().map_err(write_error)
}

fn lengths(src: &str, count_path: Option<&str>, dst: &str) -> Result<()> {
    let mut reader = FastqReader::open(src)?;
    let mut seen = vec![false; MAX_LEN];
    let mut count = 0_u64;
    while let Some(rec) = reader.next_view().map_err(read_error)? {
        let n = rec.seq.strip_suffix(b"\r").unwrap_or(&rec.seq).len();
        if n >= MAX_LEN {
            return Err(format!("ERROR: Too long read sequence found. Maximum length of supported reads is {MAX_LEN}."));
        }
        if n == 0 { return Err("Each read sequence must be at least 1 bp in length.".into()); }
        seen[n] = true;
        count += 1;
    }
    if let Some(path) = count_path {
        let mut count_out = output(path)?;
        writeln!(count_out, "{count}").map_err(write_error)?;
        count_out.flush().map_err(write_error)?;
    }
    let mut out = output(dst)?;
    for n in (0..MAX_LEN).rev() {
        if seen[n] { writeln!(out, "{n}").map_err(write_error)?; }
    }
    out.flush().map_err(write_error)
}

fn tab(src: &str, dst: &str, columns: usize) -> Result<()> {
    let mut reader = raw_input(src)?;
    let mut out = output(dst)?;
    let mut buffer = vec![0_u8; BUFFER_SIZE];
    let mut col = 0;
    let mut last = None;
    loop {
        let bytes = reader.read(&mut buffer).map_err(read_error)?;
        if bytes == 0 { break; }
        last = Some(buffer[bytes - 1]);
        for byte in &mut buffer[..bytes] {
            if *byte == b'\n' {
                col += 1;
                if col == columns { col = 0; } else { *byte = b'\t'; }
            }
        }
        out.write_all(&buffer[..bytes]).map_err(write_error)?;
    }
    if last.is_some_and(|byte| byte != b'\n') {
        col += 1;
        out.write_all(if col == columns { b"\n" } else { b"\t" })
            .map_err(write_error)?;
    }
    out.flush().map_err(write_error)
}

fn detab(src: &str, dst: &str) -> Result<()> {
    let mut reader = raw_input(src)?;
    let mut out = output(dst)?;
    let mut buf = vec![0_u8; BUFFER_SIZE];
    let mut last = None;
    loop {
        let n = reader.read(&mut buf).map_err(read_error)?;
        if n == 0 { break; }
        last = Some(buf[n - 1]);
        for byte in &mut buf[..n] { if *byte == b'\t' { *byte = b'\n'; } }
        out.write_all(&buf[..n]).map_err(write_error)?;
    }
    if last.is_some_and(|byte| byte != b'\n') {
        out.write_all(b"\n").map_err(write_error)?;
    }
    out.flush().map_err(write_error)
}

fn drop_short(n: usize, src: &str, dst: &str) -> Result<()> {
    for_each_view(src, dst, |rec, out| {
        if rec.seq.len() >= n { append_original(out, rec); }
        Ok(())
    })
}

fn ntoa(src: &str, dst: &str) -> Result<()> {
    let mut reader = raw_input(src)?;
    let mut out = output(dst)?;
    let mut buf = vec![0_u8; BUFFER_SIZE];
    let mut line = 0_u8;
    let mut last = None;
    loop {
        let n = reader.read(&mut buf).map_err(read_error)?;
        if n == 0 {
            break;
        }
        last = Some(buf[n - 1]);
        for byte in &mut buf[..n] {
            if *byte == b'\n' {
                line = (line + 1) & 3;
            } else if line == 1 && *byte == b'N' {
                *byte = b'A';
            }
        }
        out.write_all(&buf[..n]).map_err(write_error)?;
    }
    if last.is_some_and(|byte| byte != b'\n') {
        out.write_all(b"\n").map_err(write_error)?;
    }
    out.flush().map_err(write_error)
}

fn trim(n: usize, five_prime: bool, src: &str, dst: &str) -> Result<()> {
    for_each_view(src, dst, |rec, out| {
        if rec.seq.len() > n {
            let (seq, qual) = if five_prime {
                (&rec.seq[n..], &rec.qual[n.min(rec.qual.len())..])
            } else {
                (
                    &rec.seq[..rec.seq.len() - n],
                    &rec.qual[..rec.qual.len().saturating_sub(n)],
                )
            };
            append_normalized_parts(out, rec.id, seq, qual);
        } else {
            append_normalized_parts(out, rec.id, b"A", b"I");
        }
        Ok(())
    })
}

fn retain(n: usize, five_prime: bool, src: &str, dst: &str) -> Result<()> {
    for_each_view(src, dst, |rec, out| {
        if rec.seq.len() > n {
            let (seq, qual) = if five_prime {
                (&rec.seq[..n], &rec.qual[..n.min(rec.qual.len())])
            } else {
                (
                    &rec.seq[rec.seq.len() - n..],
                    &rec.qual[rec.qual.len().saturating_sub(n)..],
                )
            };
            append_normalized_parts(out, rec.id, seq, qual);
        } else {
            append_normalized_parts(out, rec.id, rec.seq, rec.qual);
        }
        Ok(())
    })
}

fn short_id(id: &mut Vec<u8>) {
    if let Some(pos) = id.iter().position(|&c| c == b' ') { id.truncate(pos); }
}

fn trim_id(src: &str, dst: &str) -> Result<()> {
    for_each_view(src, dst, |rec, out| {
        append_normalized_parts(out, short_id_slice(rec.id), rec.seq, rec.qual);
        Ok(())
    })
}

fn short_id_slice(id: &[u8]) -> &[u8] {
    &id[..id.iter().position(|&byte| byte == b' ').unwrap_or(id.len())]
}

fn trim_poly(kind: &str, minimum: usize, src: &str, dst: &str) -> Result<()> {
    let fixed = match kind { "A" | "C" | "G" | "T" | "N" => Some(kind.as_bytes()[0]), "ACGT" => None, _ => return Err("ERROR: the given nucleotide for poly tails trimming is not supported!".into()) };
    for_each_view(src, dst, |rec, out| {
        let len = rec.seq.len();
        if len != 0 {
            let left_base = fixed.unwrap_or(rec.seq[0]);
            let mut left = rec.seq.iter().take_while(|&&c| c == left_base).count();
            if left < minimum { left = 0; }
            let right_base = fixed.unwrap_or(rec.seq[len - 1]);
            let mut right = rec.seq[left..].iter().rev().take_while(|&&c| c == right_base).count();
            if right < minimum { right = 0; }
            if left + right >= len {
                append_normalized_parts(out, short_id_slice(rec.id), b"A", b"I");
                return Ok(());
            } else if left != 0 || right != 0 {
                let qleft = left.min(rec.qual.len());
                let qright = rec.qual.len().saturating_sub(right).max(qleft);
                append_normalized_parts(
                    out,
                    short_id_slice(rec.id),
                    &rec.seq[left..len - right],
                    &rec.qual[qleft..qright],
                );
                return Ok(());
            }
        }
        append_normalized_parts(out, short_id_slice(rec.id), rec.seq, rec.qual);
        Ok(())
    })
}

fn compress_id(args: &[String]) -> Result<()> {
    let mut option = "@";
    let mut count_arg: Option<&str> = None;
    let (src, dst) = match args.len() {
        4 => (&args[2], &args[3]),
        5 if is_id_option(&args[2]) => { option = &args[2]; (&args[3], &args[4]) },
        5 => { count_arg = Some(&args[2]); (&args[3], &args[4]) },
        6 => { option = &args[2]; count_arg = Some(&args[3]); (&args[4], &args[5]) },
        _ => return Err(usage().into()),
    };
    if !is_id_option(option) { return Err("ERROR: Something wrong with command line options!".into()); }
    let count = match count_arg {
        None => 500_000_u64,
        Some(value) if value.contains('.') => {
            let mut s = String::new();
            File::open(value).map_err(|_| format!("ERROR: Failed to open the file '{value}'!"))?.read_to_string(&mut s).map_err(read_error)?;
            s.split_whitespace().next().unwrap_or("0").parse().unwrap_or(0)
        }
        Some(value) => value.parse().unwrap_or(0),
    };
    if count == 0 { return Err("ERROR: Wrong number of reads!".into()); }
    let interleaved = matches!(option, "@@" | "/12" | "_12");
    let suffix = match option { "/1" | "/12" => Some(b'/'), "_1" | "_12" => Some(b'_'), "/2" => Some(b'/'), "_2" => Some(b'_'), _ => None };
    let fixed_mate = matches!(option, "/2" | "_2");
    // Match the C implementation's capacity calculation: it deliberately
    // reserves one value beyond the requested count.
    let capacity_needed = if interleaved {
        (count + 2) / 2
    } else {
        count + 1
    };
    let digits = digits_for(capacity_needed);
    let mut index = 0_u64;
    let mut mate = 0_u8;
    let mut generated_id = Vec::with_capacity(digits + 3);
    for_each_view(src, dst, |rec, out| {
        if !interleaved || mate == 0 { index += 1; }
        generated_id.clear(); generated_id.push(b'@');
        encode_id(index - 1, digits, &mut generated_id);
        if let Some(sep) = suffix {
            generated_id.push(sep);
            generated_id.push(if fixed_mate { b'2' } else if interleaved { b'1' + mate } else { b'1' });
        }
        append_normalized_parts(out, &generated_id, rec.seq, rec.qual);
        if interleaved { mate ^= 1; }
        Ok(())
    })
}

fn is_id_option(s: &str) -> bool { matches!(s, "@" | "@@" | "/1" | "_1" | "/2" | "_2" | "/12" | "_12") }

fn digits_for(mut names: u64) -> usize {
    let mut digits = 1;
    while names > ALPHABET.len() as u64 { names = (names + ALPHABET.len() as u64 - 1) / ALPHABET.len() as u64; digits += 1; }
    digits
}

fn encode_id(mut n: u64, digits: usize, out: &mut Vec<u8>) {
    let start = out.len();
    out.resize(start + digits, b'0');
    for pos in (start..start + digits).rev() { out[pos] = ALPHABET[(n % ALPHABET.len() as u64) as usize]; n /= ALPHABET.len() as u64; }
}

fn pair_key(id: &[u8]) -> &[u8] {
    let end = id.iter().position(|&c| c == b' ' || c == b'/').unwrap_or(id.len());
    &id[..end]
}

fn drop_se(src: &str, dst: &str) -> Result<()> {
    let mut reader = FastqReader::open(src)?;
    let mut out = output(dst)?;
    let (mut pending, mut current) = (Record::default(), Record::default());
    let mut has_pending = false;
    while reader.next_record(&mut current).map_err(read_error)? {
        if !has_pending {
            std::mem::swap(&mut pending, &mut current);
            has_pending = true;
        } else if pair_key(&pending.id) == pair_key(&current.id) {
            pending.write_original(&mut out).map_err(write_error)?;
            current.write_original(&mut out).map_err(write_error)?;
            has_pending = false;
        } else {
            // The previous read was unpaired. Keep this read as the next
            // candidate so adjacent mates are still found after a singleton.
            std::mem::swap(&mut pending, &mut current);
        }
    }
    out.flush().map_err(write_error)
}

fn fq2fa(src: &str, dst: &str) -> Result<()> {
    let mut id = Vec::new();
    for_each_view(src, dst, |rec, out| {
        id.clear();
        id.extend_from_slice(short_id_slice(rec.id));
        if id.first() == Some(&b'@') { id[0] = b'>'; }
        append_line(out, &id);
        append_line(out, rec.seq);
        Ok(())
    })
}

fn fa2fq(src: &str, dst: &str) -> Result<()> {
    let mut reader = FastqReader::open(src)?;
    let mut out = output(dst)?;
    let (mut id, mut seq) = (Vec::new(), Vec::new());
    while reader.next_line(&mut id).map_err(read_error)? {
        if !reader.next_line(&mut seq).map_err(read_error)? { break; }
        short_id(&mut id);
        if id.first() == Some(&b'>') { id[0] = b'@'; }
        write_line(&mut out, &id).map_err(write_error)?;
        write_line(&mut out, &seq).map_err(write_error)?;
        out.write_all(b"+\n").map_err(write_error)?;
        let quality = vec![b'I'; seq.len()];
        write_line(&mut out, &quality).map_err(write_error)?;
    }
    out.flush().map_err(write_error)
}

fn rev_com(src: &str, dst: &str) -> Result<()> {
    let mut seq = Vec::new();
    let mut qual = Vec::new();
    for_each_view(src, dst, |rec, out| {
        seq.clear();
        seq.extend(rec.seq.iter().rev().map(|byte| match byte {
            b'A' => b'T', b'C' => b'G', b'G' => b'C', b'T' => b'A', other => *other,
        }));
        qual.clear();
        qual.extend(rec.qual.iter().rev());
        append_normalized_parts(out, short_id_slice(rec.id), &seq, &qual);
        Ok(())
    })
}

fn main() -> ExitCode {
    match command(&env::args().collect::<Vec<_>>()) {
        Ok(()) => ExitCode::SUCCESS,
        Err(message) => { eprintln!("{message}"); ExitCode::from(if message.starts_with("Usage:") { 1 } else { 2 }) }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn reads_fastq_without_a_final_newline() {
        let input = Cursor::new(b"@id\nACGT\n+\nIIII".to_vec());
        let mut reader = FastqReader::new(Box::new(input));
        let mut record = Record::default();
        assert!(reader.next_record(&mut record).unwrap());
        assert_eq!(record.id, b"@id");
        assert_eq!(record.seq, b"ACGT");
        assert_eq!(record.qual, b"IIII");
    }

    #[test]
    fn reads_records_across_chunk_boundaries() {
        let input = Cursor::new(b"@one\nACGT\n+\nIIII\n@two\nNN\n+\n##\n".to_vec());
        let mut reader = FastqReader::with_capacity(Box::new(input), 7);
        let mut record = Record::default();
        assert!(reader.next_record(&mut record).unwrap());
        assert_eq!(record.id, b"@one");
        assert_eq!(record.seq, b"ACGT");
        assert!(reader.next_record(&mut record).unwrap());
        assert_eq!(record.id, b"@two");
        assert_eq!(record.qual, b"##");
        assert!(!reader.next_record(&mut record).unwrap());
    }

    #[test]
    fn id_width_reserves_the_c_implementations_extra_value() {
        assert_eq!(digits_for(62), 1);
        assert_eq!(digits_for(63), 2);
        let mut id = vec![b'@'];
        encode_id(62, 2, &mut id);
        assert_eq!(id, b"@10");
    }

    #[test]
    fn mate_key_accepts_slash_and_casava_ids() {
        assert_eq!(pair_key(b"@read/1"), pair_key(b"@read/2"));
        assert_eq!(pair_key(b"@read 1:N:0"), pair_key(b"@read 2:N:0"));
        assert_ne!(pair_key(b"@read-a/1"), pair_key(b"@read-b/2"));
    }
}
