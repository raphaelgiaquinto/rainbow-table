# rainbow_table
Cracking passwords by comparing a SHA-256 hash with plaintext passwords from wordlists. 

**For educational purposes only. I am in no way responsible for your actions.**

## How to compile

```bash
gcc rainbow.c -o rainbow -pthread -lcrypto
```

## How to use

```bash
./rainbow <path to wordlis to use> <sha256 to compare> 
```
