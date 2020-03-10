## eau2

### CS4500

### Parker Griep & Andrew Colabella

### Project 1

##### Last updated: Tuesday March 10th, 2020

#### Introduction

This project, named eau2, is a three-layered system for interacting and manipulating data
over a distributed network. On the bottom there is an individual key-value store at each
network node that exchanges data with other kv stores if needed. Above that are abstractions
for interacting with distributed data, such as distributed arrays or data frames. Lastly,
there is an application layer where the client writes code that gets distributed across the
network.

#### Architecture

The codebase is separated into 3 directories.

- client: which contains all the networking code
- store: which contains adapter and dataframe
- utils: which contains all the common components shared throughout the codebase

There is of course a separate directory for extensive testing which uses the googletest framework.

#### Implementation

###### Application

An application represents the highest-level that the user interacts with. A user programs all logic
of the application code in a subclass of Application, that switches depending on which node that
instance of the application lives on.

Applications internally hold a KVStore that can be used to acquire data across the network.

In order to write an application, a user must subclass Application and override the `run_` method.
To access which node the Application is running on, useful for switching context and assigning different
jobs for different nodes, there is another method called `size_t this_node()`

###### KVStore (Key-Value Store)

A KVStore is a mapping of Keys to Values.
Keys include the index of the node it's stored on and also a String identifier.
Values are blobs of serialized data.

The KVStore supports two functions

- `Value* get(Key* k)`
- `void put(Key* k, Value* v) // Which consumes v`

##### DataFrame

A DataFrame represents tabulated data which is internally stored across the network by columns.
The DataFrame interface provides ease of manipulation of data and abstracts that it's distributed.

Since KVStores only hold columns, DataFrame has a way of placing a column into a KVStore with
`DataFrame::fromArray(Key* key, KVStore *kv, size_t amt, <array of data>)`.

Since KVStores also hold values, there is another method for scalars.
`DataFrame::fromScalar(Key* key, KVStore *kv, <singular data item>)`

#### Use cases

##### Application

Here are examples of a Demo Application that sums up numbers from 0 to 100,000.

```{c++}
class Demo : public Application {
public:
  Key main("main",0);
  Key verify("verif",0);
  Key check("ck",0);

  Demo(size_t idx): Application(idx) {}

  void run_() override {
    switch(this_node()) {
    case 0:   producer();     break;
    case 1:   counter();      break;
    case 2:   summarizer();
   }
  }

  void producer() {
    size_t SZ = 100*1000;
    double* vals = new double[SZ];
    double sum = 0;
    for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
    DataFrame::fromArray(&main, &kv, SZ, vals);
    DataFrame::fromScalar(&check, &kv, sum);
  }

  void counter() {
    DataFrame* v = kv.waitAndGet(main);
    size_t sum = 0;
    for (size_t i = 0; i < 100*1000; ++i) sum += v->get_double(0,i);
    p("The sum is  ").pln(sum);
    DataFrame::fromScalar(&verify, &kv, sum);
  }

  void summarizer() {
    DataFrame* result = kv.waitAndGet(verify);
    DataFrame* expected = kv.waitAndGet(check);
    pln(expected->get_double(0,0)==result->get_double(0,0) ? "SUCCESS":"FAILURE");
  }
};
```

##### DataFrame

Here's an example of a DataFrame being filtered.

```
class OnlyEvens : public Rower {
  bool accept(Rower& r) { return r.get_idx() % 2 == 0; }
};

Schema s("S");
Row r(s);
DataFrame df(s);

for (int i = 0; i < 100; i++) {
  r.set(0, new String("Big bopis"));
}

OnlyEvens oe;
DataFrame* new_df = df.filter(oe); // Will get only rows with even indices.
```

#### Open questions

We currently are struggling with our network implementation and are looking
to have a one-on-one with a professor to figure out how to increase the robusticity
of our solution.

#### Status

We spent this week refactoring and abstracting our utilities code. Our biggest success yet
is refactoring our Array implementation from 5 different copies of the same code (> 1000 lines)
down to just under 200.
