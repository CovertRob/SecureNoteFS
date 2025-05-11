# System Design Doc

## Directory layout and lifecycle

- Assume the user does the following:
~~~Bash
cd /some/working/dir
./securenotefs
~~~

Program should execute the following:

1. On startup
  - Check for (or create) two subdirectories in the CWD
    - notes/ - the mount point (plaintext view)
    - data/ - the backing store (ciphertext on disk)
  - If you find a tarball (e.g. notes-data.tar.gz), extract it into data/ before mounting so previous notes reappear
2. While running
  - Mount a FUSE filesystem on notes/ that proxies all operations into data/, encrypting on writes and decrypting on reads
3. On shutdown or unmount
  - Let FUSE call your `destroy` callback or simply return from `main()`
  - Then compress the entire data/ directory into a single tarball and save in cwd
  - remove the /notes and /data directories

SecureNoteFS/                          # ← your repo root, likely in
├─ CMakeLists.txt                     # Top-level build description
├─ README.md                          # Project overview & usage
│
├─ src/                               # All C++ implementation files
│   ├─ main.cpp                       # Startup/shutdown logic:
│   │                                 # • create/verify notes/ & data/ dirs
│   │                                 # • extract any existing tarball
│   │                                 # • invoke fuse_main()
│   │                                 # • on exit, timestamp-tar data/
│   │
│   ├─ fs.cpp                         # FUSE callback implementations:
│   ├─ fs.hpp                         # • mounting, passthrough proxying
│   │                                 # • read/write routed through crypto
│   │
│   ├─ crypto.cpp                     # Encryption wrapper:
│   ├─ crypto.hpp                     # • init secretstream
│   │                                 # • encrypt/decrypt chunk APIs
│   │
│   ├─ key_manager.cpp                # Key derivation & storage:
│   ├─ key_manager.hpp                # • passphrase → Argon2id → key
│   │                                 # • load/save master key file
│   │
│   ├─ tar_manager.cpp                # Tarball packing/unpacking:
│   ├─ tar_manager.hpp                # • create timestamped tar.gz
│   │                                 # • extract tar.gz into data/
│   │                                 # • error handling/log warnings
│   │
│   └─ utils.cpp                      # Any shared helpers (e.g. filesystem path ops)
│
├─ include/                           # (optional) Public headers if you split out a library
│   └─ SecureNoteFS/                  # header namespace, e.g. SecureNoteFS/fs.hpp
│
├─ tests/                             # Unit tests (using Catch2, etc.)
│   ├─ CMakeLists.txt                 # Adds test executables
│   ├─ test_crypto.cpp                # Verify encrypt→decrypt roundtrips
│   ├─ test_key_manager.cpp           # Check KDF outputs, key file I/O
│   └─ test_tar_manager.cpp           # Ensure tar/gunzip logic works
│
└─ extras/                            # (optional) scripts, sample data, tutorial files
    └─ bigbrother.c                   # reference passthrough example

