#include <stdint.h>
#include <stddef.h>
#include "utils.h"

int abs(int n) {
  return n<0?-n:n;
}

char *strncpy(char *dest, const char *src, size_t n) {
  size_t i;
  for(i=0; i<n && src[i]!='\0'; i++) {
    dest[i] = src[i];
  }
  for(; i<n; i++)
    dest[i] = '\0';
  return dest;
}

size_t strlen(const char *s) {
  size_t i;
  for(i=0; *s!='\0'; i++, s++);
  return i;
}

size_t strnlen(const char *s, size_t n) {
  size_t i;
  for(i=0; i<n && *s!='\0'; i++, s++);
  return i;
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 && *s1 == *s2) {
    s1++; s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i;
  for(i=0; i<n && *s1 && (*s1==*s2); i++, s1++, s2++);
  return (i!=n)?(*s1 - *s2):0;
}

void *memset(void *s, int c, size_t n) {
  for(size_t i=0; i<n; i++)
    *(uint8_t *)s++ = (uint8_t)c;
  return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i;
  uint8_t *p1 = (uint8_t *)s1;
  uint8_t *p2 = (uint8_t *)s2;

  for (i = 0; i < n; i++, p1++, p2++) {
    if (*p1 != *p2)
      break;
  }
  if (i == n)
    return 0;
  else
    return *p1 - *p2;
}

void *memmove(void *dest, const void *src, size_t n) {
  if (src + n > dest) {
    src += n - 1;
    dest += n - 1;
    for(size_t i=0; i<n; i++)
      *(uint8_t *)dest-- = *(uint8_t *)src--;
  } else {
    memcpy(dest, src, n);
  }

  return dest;
}

void *memchr(const void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;
  for (size_t i=0; i<n; i++, p++) {
    if (*p == c)
      return p;
  }
  return NULL;
}

char *strchr(const char *s, int c) {
  char *p = (char *)s;

  while(*p != '\0' && *p != c)
    p++;

  if (*p == '\0')
    return NULL;
  else
    return p;
}

char *strcpy(char *dest, const char *src) {
  do {
    *dest++ = *src;
  } while (*src++ != '\0');
  return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
  size_t destlen = strlen(dest);
  size_t i;

  for (i = 0; i < n; i++) {
    dest[destlen + i] = src[i];
  }

  dest[destlen + i] = '\0';
  return dest;
}

char *strcat(char *dest, const char *src) {
  size_t destlen = strlen(dest);
  strcpy(dest + destlen, src);
  return dest;
}

int isdigit(int c) {
  return (c >= '0' && c <= '9');
}

int isspace(int c) {
  return (c == ' ' || c == '\f' || c == '\n' ||
          c == '\r' || c == '\t' || c == '\v');
}

int toupper(int c) {
  if (c >= 'a' && c <= 'z')
    return c - ('a' - 'A');
  else
    return c;
}
int tolower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c + ('a' - 'A');
  else
    return c;
}


