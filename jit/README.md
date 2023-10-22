JIT compiler

This area is under construction and is not related to the rest of the VM build structure. 
To generate all the necessary files and build tests you may use:

```bash
cd jit
mkdir ir/graph/gen
ruby ./ir/ir_parser.rb

clang++-14 -std=c++17 sample.cpp ir/graph/graph.cpp ir/graph/basic_block.cpp ir/graph/instructions.cpp -o sample

./sample
```

Note: `ir/graph/gen` directory and its contents is present in the source code for convenience and will be removed later.
