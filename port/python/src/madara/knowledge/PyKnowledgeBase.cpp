#ifndef _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_
#define _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_

#include <boost/python/detail/wrap_python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/import.hpp>
#include <boost/python/enum.hpp>
#include "madara/knowledge/KnowledgeRecord.h"
#include "madara/knowledge/KnowledgeBase.h"
#include "../../FunctionDefaults.h"

#include "capnp/schema-loader.h"
#include "capnp/schema.h"

/**
 * @file PyKnowledgeBase.cpp
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains Boost.Python mappings for the
 *   madara.knowledge.KnowledgeBase
 **/

using namespace boost::python;

class knowledge_NS {};

template<typename T>
struct DontDestruct
{
  union {
    T val;
  };

  DontDestruct(T val) : val(std::move(val)) {};
  ~DontDestruct() {}

  DontDestruct(const DontDestruct &o) : val(o.val) {}
  DontDestruct(DontDestruct &&o) : val(std::move(o.val)) {}

  DontDestruct &operator=(const DontDestruct &o)
  {
    val = o.val;
    return *this;
  }

  DontDestruct &operator=(DontDestruct &&o)
  {
    val = std::move(o.val);
    return *this;
  }

  T *operator->() { return &val; }
  const T *operator->() const { return &val; }
  T &operator*() { return val; }
  const T &operator*() const { return val; }
};

static capnp::SchemaLoader schema_loader;
static std::map<std::string, DontDestruct<object>> registered_types;
static std::map<uint64_t, const std::pair<const std::string, DontDestruct<object>> *>
  registered_tags;

#define MADARA_MEMB(ret, klass, name, args) \
  static_cast<ret (klass::*) args>(&klass::name)

template<typename T, typename I>
class_<T> define_basic_any(const char * name, const char *doc, I init)
{
  using namespace madara::knowledge;
  using namespace madara::exceptions;

#define MADARA_PYSETITEM(Key, Val) \
    .def("__setitem__", +[](T &a, Key key, Val val) { \
          a[key] = val; \
        }, "Set an element at the given index to a C++ " #Val)

#define MADARA_PYASSIGN(Type) \
    .def("assign", MADARA_MEMB(void, T, assign, (const Type &) const), \
        "Set the value to a C++ " #Type)

#define MADARA_PYSETATTR(Type) \
    .def("__setattr__", +[](T &a, const char *name, Type val) { \
          a(name) = val; \
        }, "Set a field of the given name to a C++ " #Type)

  return class_<T> (name,
    doc, init)
    .def("ref", MADARA_MEMB(AnyRef, T, ref, () const), "Reference the object "
        "with an AnyRef")
    .def("ref", MADARA_MEMB(AnyRef, T, ref, (const char *) const),
        "Get the named field as an AnyRef")
    .def("ref", MADARA_MEMB(AnyRef, T, ref, (const std::string &) const),
        "Get the named field as an AnyRef")
    .def("__getattr__", MADARA_MEMB(AnyRef, T, ref, (const char *) const),
        "Get the named field as an AnyRef")
    .def("__getattr__", MADARA_MEMB(AnyRef, T, ref, (const std::string &) const),
        "Get the named field as an AnyRef")
    .def("__getitem__", MADARA_MEMB(AnyRef, T, at, (size_t) const),
        "Get the indexed element as an AnyRef")
    .def("__getitem__", MADARA_MEMB(AnyRef, T, at, (const char *) const),
        "Get the indexed element as an AnyRef")
    .def("__getitem__", MADARA_MEMB(AnyRef, T, at, (const std::string &) const),
        "Get the indexed element as an AnyRef")
    MADARA_PYSETITEM(const char *, Any)
    MADARA_PYSETITEM(const char *, AnyRef)
    MADARA_PYSETITEM(const char *, int64_t)
    MADARA_PYSETITEM(const char *, double)
    MADARA_PYSETITEM(const char *, std::string)
    MADARA_PYSETITEM(const char *, std::vector<int64_t>)
    MADARA_PYSETITEM(const char *, std::vector<double>)
    MADARA_PYSETITEM(size_t, Any)
    MADARA_PYSETITEM(size_t, AnyRef)
    MADARA_PYSETITEM(size_t, int64_t)
    MADARA_PYSETITEM(size_t, double)
    MADARA_PYSETITEM(size_t, std::string)
    MADARA_PYSETITEM(size_t, std::vector<int64_t>)
    MADARA_PYSETITEM(size_t, std::vector<double>)
    MADARA_PYASSIGN(Any)
    MADARA_PYASSIGN(AnyRef)
    MADARA_PYASSIGN(int64_t)
    MADARA_PYASSIGN(double)
    MADARA_PYASSIGN(std::string)
    MADARA_PYASSIGN(std::vector<int64_t>)
    MADARA_PYASSIGN(std::vector<double>)
    MADARA_PYSETATTR(Any)
    MADARA_PYSETATTR(AnyRef)
    MADARA_PYSETATTR(int64_t)
    MADARA_PYSETATTR(double)
    MADARA_PYSETATTR(std::string)
    MADARA_PYSETATTR(std::vector<int64_t>)
    MADARA_PYSETATTR(std::vector<double>)
    .def("to_integer", &T::to_integer, "Convert to an integer. Zero by default")
    .def("__int__", &T::to_integer, "Convert to an integer. Zero by default")
    .def("to_double", &T::to_double, "Convert to an integer. Zero by default")
    .def("__float__", &T::to_double, "Convert to an integer. Zero by default")
    .def("to_integers", &T::to_integers, "Convert to a list of integers. "
        "Empty by default.")
    .def("to_doubles", &T::to_doubles, "Convert to a list of doubles. "
        "Empty by default.")
    .def("to_string", &T::to_string, "Convert to a string. "
        "If no other conversion is available, serialize to JSON.")
    .def("__str__", &T::to_string, "Convert to a string. "
        "If no other conversion is available, serialize to JSON.")
    .def("to_record", &T::to_record, "Convert to a KnowledgeRecord")
    .def("__bool__", +[](T &a) {
        return !a.empty() && a.template to<bool>();
      }, "False if empty, otherwise based on held object.")
    .def("size", &T::size, "Call .size() on held object")
    .def("__len__", &T::size, "Call .size() on held object")
    .def("empty", &T::empty, "Return true if there is no held object")
    .def("list_fields", +[](const T &a) {
          std::vector<std::string> ret;
          for (const auto &cur : a.list_fields()) {
            ret.emplace_back(cur.name());
          }
          return ret;
        }, "Return all registered field names as a list of strings")
    .def("tag", &T::tag, "Gets the tag of the stored object")
    .def("to_json", &T::to_json, "Serialize object as JSON")
    .def("supports_size", &T::supports_size, "Check if size() is supported")
    .def("supports_int_index", &T::supports_int_index,
        "Check if integer indexing is supported")
    .def("supports_string_index", &T::supports_string_index,
        "Check if string indexing is supported")
    .def("supports_fields", &T::supports_fields,
        "Check if fields are supported")
    .def("reader", +[](const T &a) -> object {
        auto buf = a.get_capnp_buffer().asChars();
        Py_buffer pybuf;
        int err = PyBuffer_FillInfo(&pybuf, 0, (char*)buf.begin(), buf.size(),
                                    true, PyBUF_CONTIG_RO);
        if (err == -1) {
            PyErr_Print();
            throw madara::exceptions::MadaraException("Bad python buffer");
        }
        object bytes(handle<>(PyMemoryView_FromBuffer(&pybuf)));
        //object bytes(handle<>(PyByteArray_FromStringAndSize(buf.begin(), buf.size())));
        //std::vector<unsigned char> sbuf(buf.begin(), buf.begin() + buf.size());
        return registered_types.at(a.tag())->attr("from_bytes")(bytes);
      }, "Get a Cap'n Proto reader for held object. Throws if held object "
         "isn't a Cap'n Proto message.")
  ;
}

void define_knowledge (void)
{
  object ke = object (handle<> (
    PyModule_New ("madara.knowledge")));

   ke.attr("__file__")="<synthetic>";
   scope().attr ("knowledge") = ke;
   ke.attr ("__doc__") = "Provides access to the knowledge engine";

   // this was the missing piece: sys.modules['modA.modB']=modB
   extract <dict> (getattr (
     import ("sys"), "modules"))()["madara.knowledge"] = ke;

   scope knowledge = ke;

/*
   scope knowledge
    = class_ <knowledge_NS, boost::noncopyable> (
      "knowledge_engine", no_init);
   */

  using namespace madara::knowledge;
  using namespace madara::exceptions;

  define_basic_any<AnyRef>("AnyRef",
    "A class that can refer to any type in a KnowledgeRecord", no_init);

#define MADARA_REGIMPL(name, ...) \
  .def("register_" #name, +[](const char *tag){ \
      static const std::string my_tag = tag; \
      Any::register_type<__VA_ARGS__>(my_tag.c_str());}, \
      "Register C++ type " #name ". If called multiple times, " \
      "calls after first are ignored.") \
  .staticmethod("register_" #name)

#define MADARA_REGIMPL_VECTOR(name, type) \
  MADARA_REGIMPL(name ## _vector, std::vector<type>)

  static const auto load_capn = [](object obj) {
      uint64_t id = extract<uint64_t>(
          obj.attr("schema").attr("node").attr("id"));

      std::string buf = extract<std::string>(obj.attr("to_bytes")());

      auto tag_info = registered_tags.at(id);

      auto tag = tag_info->first.c_str();

      return RegCapnObject(tag, buf.data(), buf.size());
   };

  define_basic_any<Any>("Any",
    "A class that can store any type in a KnowledgeRecord", no_init)
    .def("__init__", make_constructor(+[](object obj) {
          extract<const char *> exstr(obj);
          if (exstr.check()) {
            return std::make_shared<Any>(Any::construct(exstr()));
          }

          auto capnobj = load_capn(obj);

          return std::make_shared<Any>(std::move(capnobj));
        }), "Construct from a Cap'n Proto message object, or a string type "
            "tag previously registered with an Any.register_* function."
            "In the latter case, the object will be default constructed.")
    .def("replace", +[](Any &a, object obj) {
          extract<const char *> exstr(obj);
          if (exstr.check()) {
            a.emplace(exstr());
            return;
          }

          auto capnobj = load_capn(obj);

          a.set(std::move(capnobj));
        }, "Replace using a Cap'n Proto message object, or a string type "
           "tag previously registered with an Any.register_* function."
           "In the latter case, the object will be default constructed.")
    MADARA_REGIMPL(char, char)
    MADARA_REGIMPL(uchar, unsigned char)
    MADARA_REGIMPL(schar, signed char)
    MADARA_REGIMPL(int16, int16_t)
    MADARA_REGIMPL(int32, int32_t)
    MADARA_REGIMPL(int64, int64_t)
    MADARA_REGIMPL(uint16, uint16_t)
    MADARA_REGIMPL(uint32, uint32_t)
    MADARA_REGIMPL(uint64, uint64_t)
    MADARA_REGIMPL(float, float)
    MADARA_REGIMPL(double, double)
    MADARA_REGIMPL(string, std::string)
    MADARA_REGIMPL_VECTOR(char, char)
    MADARA_REGIMPL_VECTOR(uchar, unsigned char)
    MADARA_REGIMPL_VECTOR(schar, signed char)
    MADARA_REGIMPL_VECTOR(int16, int16_t)
    MADARA_REGIMPL_VECTOR(int32, int32_t)
    MADARA_REGIMPL_VECTOR(int64, int64_t)
    MADARA_REGIMPL_VECTOR(uint16, uint16_t)
    MADARA_REGIMPL_VECTOR(uint32, uint32_t)
    MADARA_REGIMPL_VECTOR(uint64, uint64_t)
    MADARA_REGIMPL_VECTOR(float, float)
    MADARA_REGIMPL_VECTOR(double, double)
    MADARA_REGIMPL_VECTOR(string, std::string)
    MADARA_REGIMPL(string_string_map, std::map<std::string, std::string>)
    MADARA_REGIMPL(string_int64_map, std::map<std::string, int64_t>)
    MADARA_REGIMPL(string_double_map, std::map<std::string, double>)
    .def("register_class", +[](const std::string &a, const object &klass) {
        uint64_t id = extract<uint64_t>(
            klass.attr("schema").attr("node").attr("id"));

        const std::string schema_raw = extract<std::string>(
            klass.attr("schema").attr("node")
              .attr("as_builder")().attr("to_bytes")());
        capnp::FlatArrayMessageReader msg(
            {(const capnp::word *)schema_raw.data(),
             schema_raw.size()});
        auto reader = msg.getRoot<capnp::schema::Node>();
        capnp::StructSchema schema = schema_loader.load(reader).asStruct();

        std::unique_ptr<std::string> copy(new std::string(a));
        if (Any::register_schema(copy->c_str(), schema)) {
          // Schema is registered, and name will be needed for remainder of
          // process execution. OK to leak the copy.
          copy.release();
        }

        registered_tags[id] = &*(registered_types.emplace(a,
              DontDestruct<object>(std::move(klass))).first);
      })
    .staticmethod("register_class")
  ;

  class_<madara::knowledge::KnowledgeBase> ("KnowledgeBase",
    "Network-enabled, thread-safe knowledge context", init <> ())
      
    // define constructors
    .def (init <const std::string &,
          const madara::transport::TransportSettings &> ())
    
    // define constructors
    .def (init <const madara::knowledge::KnowledgeBase &> ())

    // acquire recursive lock on KB
    .def ("acquire",
      &madara::knowledge::KnowledgeBase::acquire,
      "Acquires the recursive lock on the knowledge base. This will"
      "block any other thread from updating or using the knowledge"
      "base until you call @ release")           

    // starts the transport mechanism
    .def ("activate_transport",
      &madara::knowledge::KnowledgeBase::activate_transport,
      "Starts the transport mechanism for dissemination if it is closed")

    // apply current time to all global variables
    .def ("apply_modified",
      &madara::knowledge::KnowledgeBase::apply_modified,
      "Applies current time and modified to all global variables and tries"
       "to send them")

    // attach logger for printing
    .def ("attach_logger",
      &madara::knowledge::KnowledgeBase::attach_logger,
      "Attaches a logger to be used for printing")         

    // attach a transport to knowledge engine
    .def( "attach_transport",
      static_cast<
        size_t (madara::knowledge::KnowledgeBase::*)(
          madara::transport::Base * transport) 
      > (&madara::knowledge::KnowledgeBase::attach_transport),
         "Attaches a transport to the Knowledge Engine. Note that the"
        "transport should use the same ThreadSafeContext as the"
        "Knowledge Engine")
        
    // attach a transport to knowledge engine with specific setting
    .def( "attach_transport",
      static_cast<
        size_t (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          madara::transport::TransportSettings &) 
      > (&madara::knowledge::KnowledgeBase::attach_transport),
         "Adds a built-in transport with the specified settings")      

    // clear a variable
    .def( "clear",
      static_cast<
        bool (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::KnowledgeReferenceSettings & ) 
      > (&madara::knowledge::KnowledgeBase::clear),
        "Clears a variable. This is safer than erasing the variable."
        "It clears the memory used in the variable and marks it as UNCREATED,"
        "meaning that it is effectively deleted, will not show up in"
        "@see print statements or @see save_checkpoint")

    // Clears the whole knowledge base
    .def( "clear",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
        bool erase) 
      > (&madara::knowledge::KnowledgeBase::clear),
        "Clears the knowledge base")            

    // clear history record
    .def ("clear_history",
      static_cast<void (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::clear_history),
        "Clear all history for this record, keeping the current value") 

    // clear history record
    .def ("clear_history",
      static_cast<void (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::clear_history),
        "Clear all history for this record, keeping the current value")         

    // clear the KB
    .def ("clear_map",
      &madara::knowledge::KnowledgeBase::clear_map,
      "Clears the knowledge base")  

    // clear all modifications to KB
    .def ("clear_modifieds",
      &madara::knowledge::KnowledgeBase::clear_modifieds,
      "Clear all modifications to the knowledge base. This action may"
      "be useful if you are wanting to keep local changes but not"
      "inform other agents (possibly due to a need to further process"
      "and verify the information). The knowledge stays in its current"
      "form (i.e., this does not roll back state or anything like that)")                     

    // close the transport mechanism
    .def ("close_transport",
      &madara::knowledge::KnowledgeBase::close_transport,
      "Closes the transport mechanism so no dissemination is possible")

    // copy variables and values to a context
    .def( "copy",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::KnowledgeBase &,
          const madara::knowledge::KnowledgeRequirements &,
          const madara::knowledge::EvalSettings & ) 
      > (&madara::knowledge::KnowledgeBase::copy), 
        "Copies variables and values from source to this context."
        "PERFORMANCE NOTES: predicates with prefixes can limit"
        "copying to O(log n). predices with suffixes and no prefix"
        "force O(n) copy cost since all records could fit requirements")
        
    // copy variables and values to a context
    .def( "copy",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::KnowledgeBase &,
          const madara::knowledge::CopySet &,
          bool clean_copy,
          const madara::knowledge::EvalSettings & ) 
      > (&madara::knowledge::KnowledgeBase::copy), 
        "Copies variables and values from source to this context. PERFORMANCE"
        "NOTES: worst case depends on size of copy_set. If empty, performance"
        "is always O (n), where n is number of variables in the source context."
        "If copy_set is not empty, then performance is O (m log n) where m is"
        "the number of variables in copy_set and n is number of variables in"
        "the source context")

    // compile KaRL expression
    .def ("compile",
      &madara::knowledge::KnowledgeBase::compile,
      "Compiles a KaRL expression into an expression tree")      

    // expands a statement with variable expansion
    .def ("debug_modifieds",
      &madara::knowledge::KnowledgeBase::debug_modifieds,
      "Retrieves a stringified list of all modified variables that are ready"
      "to send over transport on next send_modifieds call")

    // defines a python function
    .def( "define_function",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          object)
      > (&madara::knowledge::KnowledgeBase::define_function),
      "defines a named function that can be called within evaluates")

    // defines a python function
    .def( "define_function",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const std::string &)
      > (&madara::knowledge::KnowledgeBase::define_function),
      "Defines a MADARA KaRL function")

    // defines a python function
    .def( "define_function",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::CompiledExpression &)
      > (&madara::knowledge::KnowledgeBase::define_function),
      "Defines a MADARA KaRL function")       

    // emplace an empty Any value within a variable
    .def( "emplace_any",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &) 
      > (&madara::knowledge::KnowledgeBase::emplace_any),
        "Atomically emplaces an empty Any value within the given variable")

    // emplace an empty Any value within a variable
    .def( "emplace_any",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &) 
      > (&madara::knowledge::KnowledgeBase::emplace_any),
        "Atomically emplaces an empty Any value within the given variable")           
        
    // evaluate an expression
    .def( "evaluate",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          const std::string &, const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::evaluate),
        m_eval_1_of_2 (
        args("expression", "settings"),
        "Evaluates an expression"))

    // evaluate an expression
    .def( "evaluate",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          madara::knowledge::CompiledExpression &,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::evaluate),
        m_eval_1_of_2 (
        args("expression", "settings"),
        "Evaluates an expression"))

    // Evaluates a root-based tree 
    .def( "evaluate",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          madara::expression::ComponentNode * root,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::evaluate),
        m_eval_1_of_2 (
        args("expression", "settings"),
        "Evaluates a root-based tree (result of compile)"))

    // load and evaluate KaRL file
    .def ("evaluate_file",
      &madara::knowledge::KnowledgeBase::evaluate_file,
      "Loads and evaluates a karl script from a file")        

    // check if a knowledge location exists
    .def( "exists",
      static_cast<
        bool (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::KnowledgeReferenceSettings & ) const
      > (&madara::knowledge::KnowledgeBase::exists), 
        "Checks if a knowledge location exists in the context")

    // check if a knowledge location exists
    .def( "exists",
      static_cast<
        bool (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeReferenceSettings & ) const
      > (&madara::knowledge::KnowledgeBase::exists),
        "Checks if a knowledge location exists in the context")

    // expands a statement with variable expansion
    .def ("expand_statement",
      &madara::knowledge::KnowledgeBase::expand_statement,
      "Expand a statement")

    // wait for a change to happen
    .def ("facade_for",
      &madara::knowledge::KnowledgeBase::facade_for,
      "Change the knowledge base to become a facade for another context."
      "It is extremely important that the context stays within scope for"
      "the duration of the life of this Knowledge Base. Otherwise, the"
      "Knowledge Base will eventually point to invalid memory")      

    // convert file to string
    .def ("file_to_string",
      &madara::knowledge::KnowledgeBase::file_to_string,
      "Loads and returns a karl script from a file with encode/decode")       
            
    // Retrieve a knowledge value
    .def( "get",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::KnowledgeBase::get),
      m_get_1_of_2 (
        args("key", "settings"),
        "Retrieves a knowledge value"))

    // get a knowledge record
    .def( "get",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::KnowledgeBase::get),
      m_get_1_of_2 (
        args("variable", "settings"),
        "Atomically returns the value of a variable."))

    // get entire stored history 
    .def ("get_history",
      static_cast<std::vector<KnowledgeRecord> (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Get a copy of the entire stored history of this record") 

    // get range of history 
    .def ("get_history",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        size_t index,
        size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Return a copy of the given range of history in a vector. Indexing"
        "starts from oldest history entry in the buffer at index 0. Negative"
        "indices count from newest entries (-1 is newest)") 

    // get history at certain index
    .def ("get_history",
      static_cast<KnowledgeRecord
       (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        size_t index,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Return the given entry in this record's history. Indexing"
        "starts from oldest history entry in the buffer at index 0. Negative"
        "indices count from newest entries (-1 is newest)")  

    // get entire history
    .def ("get_history",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Get a copy of the entire stored history of this record") 

    // get history within a range
    .def ("get_history",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        size_t index, size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Return a copy of the given range of history in a vector. Indexing"
        "starts from oldest history entry in the buffer at index 0. Negative"
        "indices count from newest entries (-1 is newest)")    

    // get history at a given entry
    .def ("get_history",
      static_cast<KnowledgeRecord
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        size_t index,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history),
        "Return the given entry in this record's history. Indexing"
        "starts from oldest history entry in the buffer at index 0. Negative"
        "indices count from newest entries (-1 is newest)")       

    // get history size
    .def ("get_history_size",
      static_cast<size_t (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history_size),
        "Return the amount of history this record holds")   

    // get history size
    .def ("get_history_size",
      static_cast<size_t (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history_size),
      "Return the amount of history this record holds")    

    // get history capacity
    .def ("get_history_capacity",
      static_cast<size_t (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history_capacity),
        "Return the maximum amount of history this record can hold. Use"
        "set_history_capacity to adjust this")      

    // get history capacity
    .def ("get_history_capacity",
      static_cast<size_t (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_history_capacity),
        "Return the maximum amount of history this record can hold. Use"
        "set_history_capacity to adjust this")        

    // return unique host
    .def ("get_id",
      &madara::knowledge::KnowledgeBase::get_id,
      "Returns the unique host and ephemeral binding for this Knowlede Base") 

    // get log level
    .def ("get_log_level",
      &madara::knowledge::KnowledgeBase::get_log_level,
      "Gets the log level")            

    // get matching variables
    .def ("get_matches",
      &madara::knowledge::KnowledgeBase::get_matches,
      "Creates an iteration of VariableReferences to all keys matching"
      "the prefix and suffix")       

    // get newest history entry for record
    .def ("get_newest",
      static_cast<KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_newest),
        "Return the newest stored history entry of this record") 

    // get newest history entry for record
    .def ("get_newest",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_newest),
        "Return the @a count newest stored history entries of this record in"
        "a vector") 

    // get newest history entry for record
    .def ("get_newest",
      static_cast<KnowledgeRecord
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_newest),
        "Return the newest stored history entry of this record")  

    // get newest history entry for record
    .def ("get_newest",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_newest),
        "Return the @a count newest stored history entries of this record in"
        "a vector")         

      // get the number of transports
    .def ("get_num_transports",
      &madara::knowledge::KnowledgeBase::get_num_transports,
      "Gets the number of transports")                    

    // get oldest history entry for record
    .def ("get_oldest",
      static_cast<KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_oldest),
        "Return the oldest stored history entry of this record") 

    // get oldest history entry for record
    .def ("get_oldest",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_oldest),
        "Return the @a count oldest stored history entries of this record in"
        "a vector") 

    // get oldest history entry for record
    .def ("get_oldest",
      static_cast<KnowledgeRecord
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_oldest),
        "Return the oldest stored history entry of this record")  

    // get oldest history entry for record
    .def ("get_oldest",
      static_cast<std::vector<KnowledgeRecord>
       (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        size_t count,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::get_oldest),
        "Return the @a count oldest stored history entries of this record in"
        "a vector")  

    // return a reference to a variable
    .def ("get_ref",
      &madara::knowledge::KnowledgeBase::get_ref,
      "Atomically returns a reference to the variable. Variable references are"
      "efficient mechanisms for reference variables individually--similar to"
      "speedups seen from CompiledExpression")        

    // see if there is history in KB
    .def ("has_history",
      static_cast<bool (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::has_history),
        "Return true if this record has a circular buffer history. Use"
        "set_history_capacity to add a buffer")         

    // see if there is history in KB
    .def ("has_history",
      static_cast<bool (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::KnowledgeBase::has_history),
        "Return true if this record has a circular buffer history. Use"
        "set_history_capacity to add a buffer")              
        
    // expands and prints a statement
    .def ("load_context",
      static_cast<int64_t (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        bool,
        const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::KnowledgeBase::load_context),
      m_load_context_1_of_3 (
        args("filename", "use_id", "settings"),
        "Loads a variable context from a file"))

    // expands and prints a statement
    .def ("load_context",
      static_cast<int64_t (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        FileHeader &,
        bool use_id,
        const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::KnowledgeBase::load_context),
      m_load_context_1_of_3 (
        args("filename", "use_id", "settings"),
        "Loads a variable context from a file"))        
          
    // expands and prints a statement
    .def ("load_context",
      static_cast<int64_t (madara::knowledge::KnowledgeBase::*)(
        madara::knowledge::CheckpointSettings &,
        const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::KnowledgeBase::load_context),
      m_load_context_1_of_2 (
        args("checkpoint_settings", "update_settings"),
        "Loads a variable context from a file with settings "
        "for checkpoint and knowledge updates"))
          
    // locks the knowledge base from updates from other threads
    .def ("lock",
      &madara::knowledge::KnowledgeBase::lock,
      "Locks the knowledge base from updates from other threads")

    // mark a variable as modified
    .def( "mark_modified",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeUpdateSettings & ) 
      > (&madara::knowledge::KnowledgeBase::mark_modified),
        "Marks the variable reference as updated")

    // mark a variable as modified
    .def( "mark_modified",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::KnowledgeUpdateSettings & ) 
      > (&madara::knowledge::KnowledgeBase::mark_modified), 
        "Marks the variable as updated")

    // same as apply_modified
    .def ("modify",
      &madara::knowledge::KnowledgeBase::modify,
      "Alias for apply_modified. @see apply_modified")        
        
    // evaluate an expression
    .def( "print",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          unsigned int) const
      > (&madara::knowledge::KnowledgeBase::print),
      m_print_0_of_1 (
        args("level"), "Prints all variables in the knowledge base"))
        
    // evaluate an expression
    .def( "print",
      static_cast<
        void (madara::knowledge::KnowledgeBase::*)(
          const std::string &, unsigned int) const
      > (&madara::knowledge::KnowledgeBase::print),
      m_print_1_of_2 (
        args("statement", "level"), "Expands and prints a statement"))

    // prints all knowledge variables
    .def ("print_knowledge",
      &madara::knowledge::KnowledgeBase::print_knowledge,
      m_print_knowledge_0_of_1 (
        args("level"),
        "Alias of print(level). Prints all variables in the knowledge base"))

    // evaluate an expression
    .def( "read_file",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const std::string &,
          const madara::knowledge::EvalSettings & ) 
      > (&madara::knowledge::KnowledgeBase::read_file),
      m_read_file_2_of_3 (
        args("knowledge_key", "filename", "settings"), 
        "Read a file into the knowledge base"))
        
    // evaluate an expression
    .def( "read_file",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const std::string &,
          const madara::knowledge::EvalSettings & ) 
      > (&madara::knowledge::KnowledgeBase::read_file),
      m_read_file_2_of_3 (
        args("variable","filename", "settings"), 
        "Read a file into the knowledge base"))

    // releases a recursive lock on KB
    .def ("release",
      &madara::knowledge::KnowledgeBase::release,
      "Releases a recursive lock on the knowledge base. This will"
      "allow other thread to access the knowledge base if you had"
      "previously called @ acquire")        

    // removes transport
    .def ("remove_transport",
      &madara::knowledge::KnowledgeBase::remove_transport,
      "Removes a transport")        

    // reset the checkpoint
    .def ("reset_checkpoint",
      &madara::knowledge::KnowledgeBase::reset_checkpoint,
      "Resets the local changed map, which tracks checkpointing modifieds")        

    // get a knowledge record at an index
    .def( "retrieve_index",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
            size_t,
            const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::KnowledgeBase::retrieve_index),
      m_retrieve_index_2_of_3 (
        args("key", "index", "settings"),
        "Retrieves a knowledge record from an index"))

    // saves the context as JSON
    .def ("save_as_json",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          const std::string &) const
      > (&madara::knowledge::KnowledgeBase::save_as_json),
        m_save_as_json_1_of_1 (
        args("settings"),
        "Saves the context to a file as JSON"))        
              
    // saves the context as JSON
    .def ("save_as_json",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::CheckpointSettings &) const
      > (&madara::knowledge::KnowledgeBase::save_as_json),
        m_save_as_json_1_of_1 (
        args("settings"),
        "Saves the context to a file as JSON"))

    // saves the context as karl
    .def ("save_as_karl",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          const std::string &) const
      > (&madara::knowledge::KnowledgeBase::save_as_karl),
        m_save_as_karl_1_of_1 (
        args("settings"),
        "Saves the context to a file as karl assignments, rather than binary"))        
         
    // saves the context as karl
    .def ("save_as_karl",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::CheckpointSettings &) const
      > (&madara::knowledge::KnowledgeBase::save_as_karl),
        m_save_as_karl_1_of_1 (
        args("settings"),
        "Saves the context to a file as karl assignments, rather than binary"))

    // saves a diff of the context as binary
    .def ("save_checkpoint",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
         const std::string &,
         bool reset_modifieds)
      > (&madara::knowledge::KnowledgeBase::save_checkpoint),
        m_save_checkpoint_1_of_1 (
        args("settings"),
        "Saves a checkpoint of a list of changes to a file"))        
           
    // saves a diff of the context as binary
    .def ("save_checkpoint",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          madara::knowledge::CheckpointSettings &) const
      > (&madara::knowledge::KnowledgeBase::save_checkpoint),
        m_save_checkpoint_1_of_1 (
        args("settings"),
        "Saves a checkpoint of a list of changes to a file"))        

    // saves the context as binary
    .def ("save_context",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          const std::string &) const
      > (&madara::knowledge::KnowledgeBase::save_context),
        m_save_context_1_of_1 (
        args("settings"),
        "Saves the context to a file"))        
         
    // saves the context as binary
    .def ("save_context",
      static_cast<int64_t
        (madara::knowledge::KnowledgeBase::*)(
          madara::knowledge::CheckpointSettings &) const
      > (&madara::knowledge::KnowledgeBase::save_context),
        m_save_context_1_of_1 (
        args("settings"),
        "Saves the context to a file with settings"))

    // Saves the list of modified records to use later for resending
    .def ("save_modifieds",
      &madara::knowledge::KnowledgeBase::save_modifieds,
      "Saves the list of modified records to use later for resending. This"
      "does not clear the modified list. This feature is useful if you"
      "want to remember what has been modified and then resend later, e.g.,"
      "if you believe packets may be dropped and want to resend information."
      "Use this function in conjunction with @see add_modifieds to remodify"
      "@return a vector of VariableReferences to the current modified list")          
          
    // Sends all modified variables through the attached transports
    .def( "send_modifieds",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::send_modifieds),
      m_send_modifieds_0_of_2 (
        args("prefix", "settings"),
        "Sends all modified variables through the attached transports."))

    // sets a knowledge record to a double
    .def( "set",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          double,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::set),
      m_set_2_of_3 (
        args("key", "value", "settings"),
        "Sets a knowledge record to a double"))
        
    // sets a knowledge record to an array of doubles
    .def( "set",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const std::vector <double> &,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::set),
      m_set_2_of_3 (
        args("key", "value", "settings"),
        "Sets a knowledge record to an array of doubles"))
        
    // sets a knowledge record to an integer
    .def( "set",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          madara::knowledge::KnowledgeRecord::Integer,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::set),
      m_set_2_of_3 (
        args("key", "value", "settings"),
        "Sets a knowledge record to an integer"))
        
    // sets a knowledge record to an array of integer
    .def( "set",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const std::vector <madara::knowledge::KnowledgeRecord::Integer> &,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::set),
      m_set_2_of_3 (
        args("key", "value", "settings"),
        "Sets a knowledge record to an array of integers"))
        
    // sets a knowledge record to a string
    .def( "set",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const std::string &,
          const madara::knowledge::EvalSettings &)
      > (&madara::knowledge::KnowledgeBase::set),
      m_set_2_of_3 (
        args("key", "value", "settings"),
        "Sets a knowledge record to a string"))

    // sets a knowledge record to an Any
    .def( "set",
        +[](KnowledgeBase &kb,
            const std::string &key,
            const Any &value,
            const madara::knowledge::EvalSettings &settings)
        {
          kb.emplace_any(key, settings, value);
        },
        "Sets a knowledge record to an Any")

    // sets a knowledge record to an Any
    .def( "set",
        +[](KnowledgeBase &kb,
            const std::string &key,
            const Any &value)
        {
          kb.emplace_any(key, value);
        },
        "Sets a knowledge record to an Any")

    // set the log level
    .def ("set_log_level",
      &madara::knowledge::KnowledgeBase::set_log_level,
      "Sets the log level")        

    // set quality of writing
    .def ("set_quality",
      &madara::knowledge::KnowledgeBase::set_quality,
      "Sets the quality of writing to a certain variable from this entity")

    // set values of a variable to a string
    .def( "set_file",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string & key,
          const unsigned char * value,
          size_t size,
          const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_file),
        "Atomically sets the value of a variable to an arbitrary string")

    // set values of a variable to a string
    .def( "set_file",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const unsigned char * value,
          size_t size,
          const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_file),
        "Atomically sets the value of a variable to an arbitrary string")

    // set history size
    .def ("set_history_capacity",
      static_cast<void (madara::knowledge::KnowledgeBase::*)(
        const std::string &,
        size_t size,
        const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_history_capacity),
        "Set the capacity of this record's history circular buffer. Every"
        "modification to this record will write a new entry in this history."
        "Once the capacity is met, the oldest entry will be discarded as new"
        "entries are added")        

    // set history size of circular buffer
    .def ("set_history_capacity",
      static_cast<void (madara::knowledge::KnowledgeBase::*)(
        const madara::knowledge::VariableReference &,
        size_t size,
        const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_history_capacity),
        "Set the capacity of this record's history circular buffer. Every"
        "modification to this record will write a new entry in this history."
        "Once the capacity is met, the oldest entry will be discarded as new"
        "entries are added")    

    // set values of a variable to a jpeg image
    .def( "set_jpeg",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const std::string &,
          const unsigned char * value,
          size_t size,
          const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_jpeg),
        "Atomically sets the value of a variable to a JPEG image")

    // set values of a variable to a jpeg image
    .def( "set_jpeg",
      static_cast<
        int (madara::knowledge::KnowledgeBase::*)(
          const madara::knowledge::VariableReference &,
          const unsigned char * value,
          size_t size,
          const madara::knowledge::EvalSettings &) 
      > (&madara::knowledge::KnowledgeBase::set_jpeg),
        "Atomically sets the value of a variable to a JPEG image")

    // set text for a file's contents
    .def ("set_text",
      &madara::knowledge::KnowledgeBase::set_text,
      "Atomically sets the value of a variable to a text file's contents")        

    // sets value of a variable to an XML string
    .def ("set_xml",
      &madara::knowledge::KnowledgeBase::set_xml,
      "Atomically sets the value of a variable to an XML string")

     // set a unique hostport
    .def ("setup_unique_hostport",
      &madara::knowledge::KnowledgeBase::setup_unique_hostport,""
      "Binds to an ephemeral port for unique tie breakers in global ordering."
      "Call this function if you want to generate a unique id for a"
      "custom transport to use (or a built-in transport if you are creating"
      "one outside of the KnowledgeBase")  

    .def("to_map",
      +[](madara::knowledge::KnowledgeBase &kb, const std::string &prefix)
      {
        return kb.to_map(prefix);
      }, "Get all records starting with the given prefix and return as "
         "KnowledgeRecordMap.")

    .def("to_map",
      +[](madara::knowledge::KnowledgeBase &kb, const std::string &prefix,
          const std::string & delimiter,
          const std::string & suffix
        )
      {
        std::vector <std::string> next_keys;
        std::map <std::string, madara::knowledge::KnowledgeRecord> result;
        kb.to_map(prefix, delimiter, suffix, next_keys, result, false);
        return result;
      },
      "Fills a variable map with list of keys according to a matching prefix,\n"
      "suffix, and delimiter hierarchy. This is useful for understanding the\n"
      "logical hierarchy of your variables\n"
      "param 0 prefix: Text that must be present at the front of the key\n"
      "param 1 delimiter: Text that signifies a logical boundary in hierarchy If\n"
      "                     empty, no check is performed.\n"
      "param 2 suffix: Text that must be present at the end of the key. If\n"
      "                     empty, no check is performed.\n"
      "returns a KnowledgeRecordMap containing the matching keys with values")


    // Write a file from the knowledge base to a specified location
    .def ("to_map_stripped",
      &madara::knowledge::KnowledgeBase::to_map_stripped,
      "Creates a map with Knowledge Records that begin with the given"
      "prefix. Runs in O(log n + m) time, where n is the size of the"
      "KnowledgeBase, and m is the number of matching records")        

    // convert a value to a string
    .def ("to_string",
      &madara::knowledge::KnowledgeBase::to_string,
      "Saves all keys and values into a string, using the underlying"
      "knowledge::KnowledgeRecord::to_string function. This is an optimized"
      "version that allows the specification of a target string to"
      "avoid string copying (which can be expensive with longer"
      "strings across multiple function boundaries). This function differs"
      "from knowledge::KnowledgeRecord to_string in that it is intended to save the"
      "database in a format that can be easily parseable. Consequently,"
      "strings are delineated in this function by being included in single"
      "quotes. Arrays are delineated with array indices")         

    // sets value of a variable to an XML string
    .def ("to_vector",
      &madara::knowledge::KnowledgeBase::to_vector,
      "Fills a vector with Knowledge Records that begin with a common subject"
      "and have a finite range of integer values") 

    // unlocks the knowledge base
    .def ("unlock",
      &madara::knowledge::KnowledgeBase::unlock,
      "Unlocks the knowledge base and allows other threads to access")

    // refer to another knowledge base
    .def ("use",
      &madara::knowledge::KnowledgeBase::use,
      "Refer to and use another knowledge base's context")      

    // wait on an expression
    .def( "wait",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          const std::string &, const madara::knowledge::WaitSettings &)
      > (&madara::knowledge::KnowledgeBase::wait),
        m_wait_1_of_2 (
        args("expression", "settings"),
        "Waits for an expression to evaluate to true"))

    // wait on an expression
    .def( "wait",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::KnowledgeBase::*)(
          madara::knowledge::CompiledExpression &,
          const madara::knowledge::WaitSettings &)
      > (&madara::knowledge::KnowledgeBase::wait),
        m_wait_1_of_2 (
        args("expression", "settings"),
        "Waits for an expression to evaluate to true"))

    // wait for a change to happen
    .def ("wait_for_change",
      &madara::knowledge::KnowledgeBase::wait_for_change,
      "Wait for a change to happen to the context (e.g., from transports)")  

    // Write a file from the knowledge base to a specified location
    .def ("write_file",
      &madara::knowledge::KnowledgeBase::write_file,
      "Write a file from the knowledge base to a specified location")               
            
  ;

  class_ <std::vector <madara::knowledge::KnowledgeRecord> > ("KnowledgeRecordVector")
    .def(vector_indexing_suite<std::vector<madara::knowledge::KnowledgeRecord> >());
  
  class_ <std::map <std::string, madara::knowledge::KnowledgeRecord> > (
    "KnowledgeRecordMap")
    .def(map_indexing_suite<std::map <
      std::string, madara::knowledge::KnowledgeRecord> >());

  // //define complex classes in the madara.knowledge module space
  // define_knowledge_base ();

  // //define madara.knowledge.containers
  // define_knowledge_containers ();

  
}

#endif // _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_

