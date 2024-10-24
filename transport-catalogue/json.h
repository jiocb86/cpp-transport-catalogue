#pragma once
 
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
 
namespace json {
 
class Node;
 
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
 
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};
 
class Node final
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;
        
    Node(Value value) : variant(std::move(value)) {}        
        
    const Array& AsArray() const {
        using namespace std::literals;
    
        if (!IsArray()) {
            throw std::logic_error("value is not an array"s);
        }
        return std::get<Array>(*this); 
    }
 
    const Dict& AsDict() const {
        using namespace std::literals;
    
        if (!IsDict()) {
            throw std::logic_error("value is not a dictionary"s);
        }
        return std::get<Dict>(*this);  
    }
 
    const std::string& AsString() const {
        using namespace std::literals;
    
        if (!IsString()) {
            throw std::logic_error("value is not a string"s);
        }
        return std::get<std::string>(*this);        
    }
    
    int AsInt() const {
        using namespace std::literals; 
    
        if (!IsInt()) {
            throw std::logic_error("value is not an int"s);
        }
        return std::get<int>(*this);    
    }
 
    double AsDouble() const {
        using namespace std::literals;
    
        if (!IsDouble()) {
            throw std::logic_error("value is not a double"s);
        } else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        return AsInt();
    }
 
    bool AsBool() const {
        using namespace std::literals;
    
        if (!IsBool()) {
            throw std::logic_error("value is not a bool"s);
        }
        return std::get<bool>(*this);   
    }
    
    bool IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }    
    
    bool IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }    

    bool IsBool() const {
        return std::holds_alternative<bool>(*this);
    }    

    bool IsString() const {
        return std::holds_alternative<std::string>(*this);
    }    

    bool IsArray() const {
        return std::holds_alternative<Array>(*this);
    }   
    
    bool IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }
 
    const Value& GetValue() const {
        return *this;
    }

    Value& GetValue() {
        return *this;
    } 
};
 
inline bool operator==(const Node& lhs, const Node& rhs) { 
    return lhs.GetValue() == rhs.GetValue();
}  
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
} 
    
class Document {
public:
    Document() = default;
    explicit Document(Node root);
    const Node& GetRoot() const;
 
private:
    Node root_;
};
 
Document Load(std::istream& input);
 
inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}
    
void Print(const Document& doc, std::ostream& output);
 
} // namespace json
