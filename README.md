# cpass
GNU Pass / gopass reimagination written in C (in development)

## Usage
 ```
 cpass init <gpg-id>
 cpass add <name> <password>
 cpass show [-c] <name>
 cpass generate [-s] [entry]
```
## Building
### Requirements
  - C compiler
  - gpgme
  - GNU Make

### Using Nix
  ```
  nix develop
  make
  ```
