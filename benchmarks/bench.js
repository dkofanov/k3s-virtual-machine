/*
objects.k3s implemntation. Produces dump in a form that can be compared with
k3s array dump via diff.*/


class Bar {
  constructor(a) {
    this.a = a;
  }
}

class Foo {
  constructor(x) {
    this.x = x;
    this.y = null;
  }

  setY(b) {
    this.y = b;
  }
}

function func(N, M) {
  var foo, o1, o2, outer;
  foo = new Array(M);
  outer = null;

  for (var i = 1; i < N + 1; i += 1) {
    o1 = new Foo(i);

    if (i % 3 === 0) {
      foo[i % M] = o1;
    }

    o2 = new Bar(i);

    if (i % 5 === 0) {
      o1.setY(o2);
    }

    outer = o1;
  }

}

function main() {
  var M, N;
  N = 4000000;
  M = 1000;
  func(N, M);
}

main();
