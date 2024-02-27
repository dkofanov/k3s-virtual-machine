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
      @props = data.props == nil ? [] : data.props
    end

    def base_class_name
      inputs_base
    end
    def inputs_base
      is_fixed? ? "FixedInputsInst<#{@argc}>" : "VariadicInputsInst"
    end
  
    def append_argc(is_decl = true)
      str = ""
      if !is_fixed?
        str += "#{is_decl ? "size_t" : ""} inputs_count"
        str += ", " if (mixins.length > 0) || !is_decl
      end
      str       
    end

    def inheritance_list
      arr = []
      arr.append "public #{base_class_name}"
      mixins.each do |mixin|
        arr.append "public #{mixin}Mixin"
      end
      arr.join(', ')
    end

    def opcode
      @opcode
    end

    def is_fixed?
      @argc != -1
    end
    
    def no_dst?
      @props.include? "no_dst"
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

IR = IRDesc.new(YAML.load_file("ir/ir.yaml"))
t_inst_inl = ERB.new(File.read("ir/graph/inst.inl.erb"), 0, "%-")
t_instructions_inl = ERB.new(File.read("ir/graph/instructions.inl.erb"), 0, "%-")
t_graph_ctor = ERB.new(File.read("ir/graph/graph_ctor.h.erb"), 0, "%-")

File.write("ir/graph/gen/inst.inl", t_inst_inl.result)
File.write("ir/graph/gen/instructions.inl", t_instructions_inl.result)
File.write("ir/graph/gen/graph_ctor.h", t_graph_ctor.result)
