#include <assert.h>
#include <gpgme.h>
#include <linux/limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char storeLocation[PATH_MAX];

void printHelp(const char *name);
void init(const char *id);
char *generatePassword(int special, int size);
void addEntry(const char *name, const char *pass);

int main(int argc, char **argv) {
  if (argc < 2) {
    printHelp(argv[0]);
    return 0;
  }

  const char *home = getenv("HOME");
  const char *customLocation = getenv("STOREPATH");
  if (!customLocation) {
    snprintf(storeLocation, PATH_MAX, "%s/.cpass", home);
  }
  else {
    strcpy(storeLocation, customLocation);
  }

  setlocale(LC_ALL, "");
  gpgme_check_version(NULL);
  
  if (argc == 2 || (strcmp(argv[1], "show") == 0) || (strcmp(argv[1], "-c") == 0)) {
    int copy = 0;
    const char *entryName = NULL;
    
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-c") == 0)
        copy = 1;
      else if (strcmp(argv[i], "show") != 0)
        entryName = argv[i];
    }
    
    if (!entryName)
      return 1;
        
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s.gpg", storeLocation, entryName);

    gpgme_ctx_t ctx;
    gpgme_data_t in, out;
    gpgme_error_t err;

    err = gpgme_new(&ctx);
    if (err)
      return 1;

    gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);

    err = gpgme_data_new_from_file(&in, path, 1);
    if (err)
      return 1;

    gpgme_data_new(&out);

    err = gpgme_op_decrypt(ctx, in, out);
    if (err)
      return 1;

    gpgme_data_seek(out, 0, SEEK_SET);

    char buf[1024];
    ssize_t n;
    if (copy) {
      FILE *pipe;
      pipe = (getenv("WAYLAND_DISPLAY") ? popen("wl-copy", "w") : popen("xclip -selection clipboard", "w"));
      if (pipe) {
        while ((n = gpgme_data_read(out, buf, sizeof(buf))) > 0)
          fwrite(buf, 1, n, pipe);
        pclose(pipe);
      } else {
        fprintf(stderr, "Error: Failed to copy entry\n");
      }
    } else {
      while ((n = gpgme_data_read(out, buf, sizeof(buf))) > 0)
        fwrite(buf, 1, n, stdout);
      putchar('\n');
    }
    gpgme_data_release(in);
    gpgme_data_release(out);
    gpgme_release(ctx);

  } else if (strcmp(argv[1], "generate") == 0) {
    int special = 0;
    int size = 12;

    if (argc > 2 && strcmp(argv[2], "-s") == 0)
      special = 1;

    char *password = generatePassword(special, size);
    if (!password)
      return 1;
    
    if (special && argc < 4)
      printf("%s\n", password);
    else
      if (special)
        addEntry(argv[3], password);
      else 
        addEntry(argv[2], password);
    free(password);

  } else if (strcmp(argv[1], "init") == 0) {
    if (argc < 3)
      return 1;

    init(argv[2]);

  } else if (strcmp(argv[1], "add") == 0) {
    if (argc < 4)
      return 1;

    addEntry(argv[2], argv[3]);
  }

  return 0;
}

void printHelp(const char *name) {
  printf("Usage:\n");
  printf(" %s init <gpg-id>\n", name);
  printf(" %s add <name> <password>\n", name);
  printf(" %s show <name>\n", name);
  printf(" %s generate [-s]\n", name);
}

void init(const char *id) {
  char dir[PATH_MAX];
  char path[PATH_MAX];

  snprintf(dir, sizeof(dir), "%s", storeLocation);
  mkdir(dir, 0700);

  snprintf(path, sizeof(path), "%s/.keyId", storeLocation);
  
  FILE *f;

  f = fopen(path, "r");
  if (f) {
    fprintf(stderr, "ERROR: %s already contains keyId file!\n", path);
    exit(1);
  }

  f = fopen(path, "w");
  if (!f)
    return;

  fprintf(f, "%s\n", id);
  fclose(f);
}

char *generatePassword(int special, int size) {
  int lowRange = (special) ? 33 : 65;
  char *buf = malloc(size + 1);
  if (!buf)
    return NULL;

  for (int i = 0; i < size; i++) {
    int aux = arc4random() % 2;
    if (aux)
      buf[i] = (arc4random() % (90 - lowRange + 1)) + lowRange;
    else
      buf[i] = (arc4random() % (122 - 97 + 1)) + 97;
  }

  buf[size] = '\0';
  return buf;
}

void addEntry(const char *name, const char *pass) {
  char entryPath[PATH_MAX];
  char keyPath[PATH_MAX];

  snprintf(entryPath, sizeof(entryPath), "%s/%s.gpg", storeLocation,
           name);

  snprintf(keyPath, sizeof(keyPath), "%s/.keyId", storeLocation);

  FILE *f = fopen(keyPath, "r");
  if (!f)
    return;

  char id[256];
  if (!fgets(id, sizeof(id), f)) {
    fclose(f);
    return;
  }
  fclose(f);

  id[strcspn(id, "\n")] = 0;

  gpgme_ctx_t ctx;
  gpgme_key_t recp[2] = {NULL, NULL};
  gpgme_data_t in, out;

  if (gpgme_new(&ctx))
    return;

  gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);

  if (gpgme_get_key(ctx, id, &recp[0], 0)) {
    gpgme_release(ctx);
    return;
  }

  gpgme_data_new_from_mem(&in, pass, strlen(pass), 0);
  gpgme_data_new(&out);

  gpgme_op_encrypt(ctx, recp, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);

  gpgme_data_seek(out, 0, SEEK_SET);
  FILE *outFile = fopen(entryPath, "wb");
  if (outFile) {
    char buf[4096];
    ssize_t n;
    while ((n = gpgme_data_read(out, buf, sizeof(buf))) > 0)
      fwrite(buf, 1, n, outFile);

    fclose(outFile);
  }

  gpgme_key_unref(recp[0]);
  gpgme_data_release(in);
  gpgme_data_release(out);
  gpgme_release(ctx);
}
