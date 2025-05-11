/*
Responsibilities of key_manager:

On first run: prompt user for passphrase, run Argon2id, save key material (~/.securenotefs/key).

On subsequent runs: load key file, unlock stream key.

Securely erase passphrase from memory once the key is derived.

*/