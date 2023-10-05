require 'erb'
require 'yaml'
require 'ostruct'

class IRDesc
  class InstDesc
    def initialize(yaml_hash)
      data = OpenStruct.new(yaml_hash)
      raise "no opcode" if !data.opcode
      @opcode = data.opcode
      raise "'argc' should be non-negative if specified" if data.argc && data.argc < 0
      @argc = data.argc == nil ? -1 : data.argc
      @mixins = data.mixins == nil ? [] : data.mixins 
    end

    def base_class_name
      is_special? ? "Inst" : inputs_base
    end
    def inputs_base
      is_fixed? ? "FixedInputsInst<#{@argc}>" : "VariadicInputsInst"
    end

    def inheritance_list
      arr = []
      arr.append "public #{base_class_name}"
      mixins.each do |mixin|
        arr.append "public #{mixin}Mixin"
      end
      arr.join(', ')
    end

    def is_special?
      @special
    end

    def opcode
      @opcode
    end

    def is_fixed?
      @argc != -1
    end

    def argc
      @argc
    end

    def mixins
      @mixins
    end
  end

  def initialize(yaml)
    @instructions = yaml["instructions"]
    @instructions.each_with_index do |inst, idx|
      puts inst
      @instructions[idx] = InstDesc.new(inst)
    end
  end
  
  def instructions 
    @instructions
  end

end

def AppendSuffix(list, suffix, sep=", ")
  return list.collect{ |x| x.to_s + suffix.to_s }.join(sep)
end
def ArgList(prefix, count)
    return (0..count - 1).collect{ |x| prefix.to_s + x.to_s }.join(", ")
end
def TypedArgList(prefix1, prefix2, count)
    return (0..count - 1).collect{ |x| prefix1.to_s + x.to_s + ' ' + prefix2.to_s + x.to_s }.join(", ")
end

IR = IRDesc.new(YAML.load_file("ir.yaml"))
template1 = ERB.new(File.read("inst_ctors.h.erb"), 0, "%-")
template2 = ERB.new(File.read("inst.h.erb"), 0, "%-")

File.write("inst_ctors_gen.h", template1.result)
File.write("inst_gen.h", template2.result)
