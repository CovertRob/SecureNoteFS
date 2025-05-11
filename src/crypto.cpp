/*
Responsibilities of crypto: 

Wrap libsodium’s secretstream API:

    init_encrypt_stream(key), push(plain) → cipher, auto-appended tag
    init_decrypt_stream(key, header), pull(cipher) → plain or auth-fail
    Keep all raw bytes in std::vector<uint8_t> or similar.
    
*/