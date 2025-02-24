set shell := ["zsh", "-uc"]

cmd := "./target/bin/Moer"

default:
    @just --list

build:
    @cmake -B build -S .
    @cmake --build build --parallel 4

test target: build
    {{cmd}} {{target}}
