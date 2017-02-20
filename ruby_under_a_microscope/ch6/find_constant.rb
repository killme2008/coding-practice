class SuperClass
  FIND_ME = "Found in Superclass"
end

module ParentLexicalScope
  FIND_ME = "Found in ParentLexicalScope"

  module ChildLexicalScope

    class SubClass < SuperClass
      p FIND_ME
    end
  end
end
