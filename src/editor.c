#include "../include/editor.h"
#include "../include/utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

char *load_file(const char *NameFile) {
  if (NameFile == NULL) {
    NameFile = "newfile";
  }

  FILE *file = fopen(NameFile, "rb");
  if (file == NULL) {
    // file does not exist: creates and returns an empty string
    file = fopen(NameFile, "wb");
    if (file == NULL) {
      perror("ERROR CREATING FILE");
      return NULL;
    }
    fclose(file);

    // return empty string
    char *empty = (char *)malloc(1);
    if (empty == NULL) {
      perror("ERROR ALLOCATING MEMORY");
      return NULL;
    }
    empty[0] = '\0';
    return empty;
  }

  // reading file
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(size + 1);
  if (buffer == NULL) {
    perror("ERROR ALLOCATING MEMORY");
    fclose(file);
    return NULL;
  }

  size_t readed = fread(buffer, 1, size, file);
  buffer[readed] = '\0'; // adding null terminator

  fclose(file);
  return buffer;
}

void set_raw_mode(int enable) {
  static struct termios oldt, newt;
  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_iflag &= ~(IXON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
}

void save_file(const char *nome_arquivo, const char *conteudo) {
  FILE *fp = fopen(nome_arquivo, "w");
  if (!fp) {
    perror("ERROR SAVING FILE");
    return;
  }
  fputs(conteudo, fp);
  fclose(fp);
}

// ANSI escape codes for clearing and moving the cursor
void render_buffer(const char *buffer, size_t cursor) {
  // clean screen
  printf("\033[H\033[J");

  // print content
  printf("%s", buffer);

  // calculate the cursor row and column
  size_t linha = 0, coluna = 0;
  for (size_t i = 0; i < cursor; i++) {
    if (buffer[i] == '\n') {
      linha++;
      coluna = 0;
    } else {
      coluna++;
    }
  }

  // move the cursor to the correct position
  printf("\033[%zu;%zuH", linha + 1, coluna + 1); // ANSI use 1-based indexing

  fflush(stdout);
}

void edit_buffer(char *text, const char *NameFile) {
  char *FileName = (char *)NameFile;
  size_t len = strlen(text);
  size_t capacity = len + 512;
  char *buffer = malloc(capacity);
  if (!buffer) {
    perror("Erro ao alocar memória");
    return;
  }

  strcpy(buffer, text);
  size_t cursor = len;

  set_raw_mode(1);
  render_buffer(buffer, cursor);

  char seq[3]; // for arrows
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {

    if (c == 27) { // ESC sequence
      // make reading non-blocking
      int old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

      usleep(10000); // wait (10ms)

      int n1 = read(STDIN_FILENO, &seq[0], 1);
      int n2 = read(STDIN_FILENO, &seq[1], 1);

      fcntl(STDIN_FILENO, F_SETFL, old_flags); // restores original mode

      if (n1 == 1 && n2 == 1 && seq[0] == '[') {
        if (seq[1] == 'C') { // > arrow right
          if (buffer[cursor] != '\0')
            cursor++;
        } else if (seq[1] == 'D') { // < arrow left
          if (cursor > 0)
            cursor--;
        } else if (seq[1] == 'A') { // ^ arrow up
          size_t i = cursor, linha_atual_inicio = 0;
          while (i > 0) {
            if (buffer[i - 1] == '\n') {
              linha_atual_inicio = i;
              break;
            }
            i--;
          }
          if (linha_atual_inicio > 0) {
            size_t linha_anterior_fim = linha_atual_inicio - 1;
            size_t linha_anterior_inicio = 0;
            for (i = linha_atual_inicio - 1; i > 0; i--) {
              if (buffer[i - 1] == '\n') {
                linha_anterior_inicio = i;
                break;
              }
            }
            size_t coluna = cursor - linha_atual_inicio;
            size_t tam_linha_anterior =
                linha_anterior_fim - linha_anterior_inicio;
            cursor =
                linha_anterior_inicio +
                (coluna < tam_linha_anterior ? coluna : tam_linha_anterior);
          }
        } else if (seq[1] == 'B') { // v arrow down
          size_t i = cursor;
          while (buffer[i] != '\0' && buffer[i] != '\n')
            i++;
          if (buffer[i] == '\n') {
            size_t proxima_linha_inicio = i + 1;
            size_t coluna = cursor - (cursor > 0 && buffer[cursor - 1] == '\n'
                                          ? cursor
                                          : i - (cursor - i));
            size_t j = proxima_linha_inicio;
            size_t linha_len = 0;
            while (buffer[j + linha_len] != '\0' &&
                   buffer[j + linha_len] != '\n')
              linha_len++;
            cursor = proxima_linha_inicio +
                     (coluna < linha_len ? coluna : linha_len);
          }
        }
      } else {
        // ESC : exit of editor
        break;
      }
    } else if (c == 19) { // Ctrl+S
      if (FileName == NULL || strlen(FileName) == 0) {
        FileName = malloc(256);
        if (!FileName) {
          perror("ERROR ALLOCATING MEMORY FOR FILE NAME");
          break;
        }

        set_raw_mode(0); // temporarilly turn off raw mode
        printf("\nEnter file Name: ");
        fflush(stdout);
        if (fgets(FileName, 256, stdin) != NULL) {
          // Remove newline, if exist
          FileName[strcspn(FileName, "\n")] = '\0';
        } else {
          fprintf(stderr, "ERROR READING FILE NAME\n");
          free(FileName);
          FileName = NULL;
          set_raw_mode(1);
          continue;
        }
        set_raw_mode(1); // turn on raw mode
      }
      save_file(FileName, buffer);
    } else if (c == 127 || c == 8) { // Backspace
      if (cursor > 0) {
        memmove(&buffer[cursor - 1], &buffer[cursor],
                strlen(&buffer[cursor]) + 1);
        cursor--;
      }
    } else if (c == '\t') { // Tab
      const int tab_size = 4;
      if (strlen(buffer) + tab_size >= capacity) {
        capacity *= 2;
        buffer = realloc(buffer, capacity);
        if (!buffer) {
          perror("ERROR REALLOCATING MEMORY");
          break;
        }
      }
      memmove(&buffer[cursor + tab_size], &buffer[cursor],
              strlen(&buffer[cursor]) + 1);
      for (int i = 0; i < tab_size; i++) {
        buffer[cursor + i] = ' ';
      }
      cursor += tab_size;
    } else if (c == '\n' || c == '\r') {
      if (strlen(buffer) + 1 >= capacity) {
        capacity *= 2;
        buffer = realloc(buffer, capacity);
        if (!buffer) {
          perror("ERROR REALLOCATING MEMORY #2");
          break;
        }
      }
      memmove(&buffer[cursor + 1], &buffer[cursor],
              strlen(&buffer[cursor]) + 1);
      buffer[cursor++] = '\n';
    } else if (c >= 32 && c <= 126) {
      if (strlen(buffer) + 1 >= capacity) {
        capacity *= 2;
        buffer = realloc(buffer, capacity);
        if (!buffer) {
          perror("ERROR REALLOCATING MEMORY #3");
          break;
        }
      }
      memmove(&buffer[cursor + 1], &buffer[cursor],
              strlen(&buffer[cursor]) + 1);
      buffer[cursor++] = c;
    } else if (c == 3) { // Ctrl+C
    }

    render_buffer(buffer, cursor);
  }

  set_raw_mode(0);
  clean_screen();
  free(buffer);
}
