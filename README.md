# FSSH - Fluent Secure Shell

A simple terminal-based SSH profile manager written in C++.

## Features

- Add, edit and delete SSH profiles
- Check host availability
- Connect to saved profiles via SSH
- Generate SSH keys
- Store profiles in a YAML configuration file

## Requirements

- C++17
- GNU Make
- Boost
- yaml-cpp
- OpenSSH
- netcat

## Build

```bash
git clone https://github.com/cheyenne-dev/fssh.git
cd fssh
make
```

## Usage

```bash
./fssh
```

## License

This project is licensed under the MIT License. See the LICENSE file for details
