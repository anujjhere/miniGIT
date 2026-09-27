# minigit

A small, from-scratch reimplementation of git's core, written in C++.

**Status:** `init`, `hash-object`, `cat-file`, `add`, `commit`, and `log`
are all implemented and tested end to end.

## What it does

- `minigit init` — creates a new repository
- `minigit hash-object <file> [-w]` — hashes a file's content (optionally stores it)
- `minigit cat-file <hash>` — prints a stored object's content
- `minigit add <file>` — stages a file for the next commit
- `minigit commit -m "message"` — commits staged files
- `minigit log` — shows commit history

Object hashing and storage follow git's own format (`blob <size>\0<content>`),
so hashes match real `git hash-object` output for the same file.

## Building

    g++ -std=c++17 src/main.cpp src/commands.cpp -o minigit

## Usage

    minigit init
    minigit add readme.txt
    minigit commit -m "first commit"
    minigit log

## Design notes

- **Object format matches real git.** Every object is stored as
  `<type> <size>\0<content>`, then SHA-1 hashed, so hashes line up
  exactly with `git hash-object` for the same content.
- **SHA-1 implemented from scratch** (`sha1.h`), with no external
  crypto library.
- **No `std::filesystem`.** Built against an older GCC (6.3) that
  lacks C++17's filesystem library, so folder handling uses the
  plain `sys/stat.h` / `_mkdir` APIs instead — a deliberate
  portability choice, not an oversight.
- **Content-addressed storage.** Two files with identical content
  hash to the same object and are stored only once.
- **The index is a flat key-value file**, loaded into a `std::map`
  on `add`, so re-adding a changed file replaces its entry instead
  of duplicating it.
- **Commits form a real DAG** — each commit stores a `tree` hash and
  an optional `parent` hash, and `log` walks that chain backward
  from `refs/heads/main`.

## Why I built this

To understand how git actually works under the hood — content-addressed
storage, the commit DAG, and how a hash uniquely represents a snapshot.