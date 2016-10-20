#include <stdio.h>

struct interface_vtable;

struct abstract {
  struct interface_vtable *vtable;
};

struct interface_vtable {
  int (*function1)(struct abstract*, const char*);
  int (*function2)(struct abstract*, int);
};


struct A {
  struct abstract interface;
  const char *prepend;
};

int A_function1(struct abstract *self, const char *msg) {
  struct A *a = (struct A *)self;

  return printf("A (prepend: %s) function1: %s\n", a->prepend, msg);
}
int A_function2(struct abstract *self, int integer) {
  struct A *a = (struct A *)self;

  return printf("A (prepend: %s) function2: %d\n", a->prepend, integer);
}

static struct interface_vtable A_vtable = {
  &A_function1,
  &A_function2
};


struct B {
  struct abstract interface;
  int *data;
};

int B_function1(struct abstract *self, const char *msg) {
  struct B *b = (struct B *)self;

  printf("(B function1) Hey %s!, here is a 5x5 matrix:\n", msg);

  int i, j;
  for(i = 0; i < 5; ++i) {
    for(j = 0; j < 5; ++j) {
      printf("%d ", b->data[i*5 + j]);
    }
    printf("\n");
  }

  return 0;
}
int B_function2(struct abstract *self, int integer) {
  struct B *b = (struct B *)self;

  printf("%d times hello!: ", integer);

  int i;
  for(i = 0; i < integer; ++i) {
    printf("hello ");
  }
  printf("\n");

  return 0;
}

static struct interface_vtable B_vtable = {
  &B_function1,
  &B_function2
};


int function1(struct abstract *self, const char *msg) {
  return self->vtable->function1(self, msg);
}
int function2(struct abstract *self, int integer) {
  return self->vtable->function2(self, integer);
}


int main() {
  struct A a;
  a.interface.vtable = &A_vtable;
  a.prepend = "GG gurl!!!";

  struct B b;
  b.interface.vtable = &B_vtable;
  int data[] = {
    1, 0, 0, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 1, 0,
    0, 0, 0, 0, 1
  };
  b.data = data;

  struct abstract *abstracts[] = {
    &a.interface, &b.interface
  };

  function1(abstracts[0], "Kim Kardashian");
  function1(abstracts[1], "Kim Kardashian");

  return 0;
}
