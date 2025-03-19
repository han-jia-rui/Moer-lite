set shell := ["zsh", "-uc"]

cmd := "./target/bin/Moer"

default:
    @just --list

release:
    @cmake -B build/release -S . -DCMAKE_BUILD_TYPE=Release
    @cmake --build build/release --parallel 8

debug:
    @cmake -B build/debug -S . -DCMAKE_BUILD_TYPE=Debug
    @cmake --build build/debug --parallel 8

test target: release
    {{cmd}} {{target}}

valgrind target: debug
    @valgrind --tool=callgrind {{cmd}} {{target}}
    @kcachegrind callgrind.out.*

clean:
   @rm -rf ./build ./target 

edit:
    $EDITOR justfile
