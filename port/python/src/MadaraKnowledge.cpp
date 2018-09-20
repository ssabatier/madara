#ifndef _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_
#define _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_

#include <boost/python/detail/wrap_python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/import.hpp>
#include <boost/python/enum.hpp>

#include "madara/knowledge/KnowledgeBase.h"
#include "madara/knowledge/FileFragmenter.h"
#include "madara/knowledge/AnyRegistry.h"
#include "madara/filters/GenericFilters.h"
#include "FunctionDefaults.h"
#include "MadaraKnowledgeContainers.h"
#include "MadaraKnowledge.h"
#include "madara/knowledge/PyKnowledgeBase.h"

#include "capnp/schema-loader.h"
#include "capnp/schema.h"

/**
 * @file MadaraModule.cpp
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains Boost.Python mappings for the madara.knowledge module
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

  class_<madara::knowledge::CheckpointSettings> ("CheckpointSettings",
    "Settings for checkpoints and file saves", init <> ())

     // class members
    .def_readwrite("buffer_filters",
      &madara::knowledge::CheckpointSettings::buffer_filters,
      "buffer filters. Note that the user must clean up memory of all filters")

    .def_readwrite("buffer_size",
      &madara::knowledge::CheckpointSettings::buffer_size,
      "size of the buffer needed to hold file in memory")

    .def_readwrite("clear_knowledge",
      &madara::knowledge::CheckpointSettings::clear_knowledge,
      "If true, during loads, clear the KnowledgeBase first")

    .def_readwrite("filename",
      &madara::knowledge::CheckpointSettings::filename,
      "the path and name of the file to load/save")

    .def_readwrite("ignore_header_check",
      &madara::knowledge::CheckpointSettings::ignore_header_check,
      "If true, do not perform a header check. This is useful if you are"
       "loading an arbitrary text file with no buffer filters")      

    .def_readwrite("initial_lamport_clock",
      &madara::knowledge::CheckpointSettings::initial_lamport_clock,
      "initial lamport clock saved in the checkpoint ")

    .def_readwrite("initial_state",
      &madara::knowledge::CheckpointSettings::initial_state,
      "initial state of interest (useful for loading ranges of checkpoints). "
      " Thsi is an inclusive identifier, so 0 means to load from initial"
      "context/checkpoint save")

    .def_readwrite("initial_timestamp",
      &madara::knowledge::CheckpointSettings::initial_timestamp,
      "initial wallclock time saved in the checkpoint")

    .def_readwrite("keep_open",
      &madara::knowledge::CheckpointSettings::keep_open,
      "attempt to keep the file open for later usage (not really recommended)")

    .def_readwrite("last_lamport_clock",
      &madara::knowledge::CheckpointSettings::last_lamport_clock,
      "final lamport clock in the checkpoint")

    .def_readwrite("last_state",
      &madara::knowledge::CheckpointSettings::last_state,
      "the last state of interest (useful for loading ranges of checkpoints). "
      "This is an inclusive identifier. If last_state >= states, it will "
      "essentially indicate the last valid state")

    .def_readwrite("last_timestamp",
      &madara::knowledge::CheckpointSettings::last_timestamp,
      "final wallclock time saved in the checkpoint")

    .def_readwrite("originator",
      &madara::knowledge::CheckpointSettings::originator,
      "the originator id of the checkpoint (who saved it)")

    .def_readwrite("override_lamport",
      &madara::knowledge::CheckpointSettings::override_lamport,
      "use the lamport clocks in this class instead of the KB clock when"
      "writing context or checkpoints")

    .def_readwrite("override_timestamp",
      &madara::knowledge::CheckpointSettings::override_timestamp,
      "use the timestamps in this class instead of wallclock time when"
      "writing context or checkpoints")

    .def_readwrite("playback_simtime",
      &madara::knowledge::CheckpointSettings::playback_simtime,
      "If true, update simtime during playback to match recorded TOI. Only"
       "operable if MADARA_FEATURE_SIMTIME macro is defined")      

    .def_readwrite("prefixes",
      &madara::knowledge::CheckpointSettings::prefixes,
      "A list of prefixes to save/load. If empty, all prefixes are valid")

    .def_readwrite("reset_checkpoint",
      &madara::knowledge::CheckpointSettings::reset_checkpoint,
      "if true, resets the checkpoint to start a new diff from this point on")

    .def_readwrite("states",
      &madara::knowledge::CheckpointSettings::states,
      "the number of states checkpointed in the file stream")

    .def_readwrite("variables_lister",
      &madara::knowledge::CheckpointSettings::variables_lister,
      "Object which will be used to extract variables for checkpoint saving."
       "By default (if left nullptr), use a default implementation which uses"
       "ThreadSaveContext::get_local_modified")

    .def_readwrite("version",
      &madara::knowledge::CheckpointSettings::version,
      "the MADARA version used when the checkpoint was saved") 
  ;

  class_<madara::knowledge::FileFragmenter> (
     "FileFragmenter",
     "Splits or reforms a file from disk or knowledge base", init <> ())

    // create a vector in the KB
    .def( "create_vector",
      &madara::knowledge::FileFragmenter::create_vector,
        m_create_vector_2_of_3 (
        args("key", "kb", "settings"),
        "Creates a Vector in the KB using current file fragments"))  

    // split a file
    .def( "fragment_buffer",
      &madara::knowledge::FileFragmenter::fragment_buffer,
        m_fragment_buffer_2_of_3 (
        args("buffer", "size", "settings"),
        "Splits a file, given a buffer and size with settings"))           

    // split a file, given a filename
    .def( "fragment_file",
      &madara::knowledge::FileFragmenter::fragment_file,
        m_fragment_file_1_of_2 (
        args("filename", "settings"),
        "Splits a file, given a filename and settings"))
      
    // evaluate an expression
    .def( "from_kb",
      &madara::knowledge::FileFragmenter::from_kb,
        m_from_kb_2_of_3 (
        args("key", "kb", "settings"),
        "Attempts to create a file from a Vector of fragments. The "
        "result is the actual number of bytes available. The result "
        "should be checked against the actual size of the file for "
        "progress information (e.g. of transfer)."))
      
    .add_property("file_contents",
      &madara::knowledge::FileFragmenter::get_file_contents,
      "Returns the merged file contents (should run from_kb first)")

     // class members

    .def_readonly("file_size",
      &madara::knowledge::FileFragmenter::file_size,
      "size of file contents")

  ;


  class_<madara::knowledge::KnowledgeRecord> (
     "KnowledgeRecord",
     "Basic unit of knowledge", init <> ())

     // integer constructor
     .def (init <int64_t> ())

     .def (init <double> ())

     .def (init <std::vector <int64_t> > ())

     .def (init <std::vector <double> > ())

     .def (init <std::string> ())

     .def (init <const Any &> ())

     .def (init <const madara::knowledge::KnowledgeRecord &> ())

    // apply knowledge record to a context
    .def ("apply", &madara::knowledge::KnowledgeRecord::apply,
      "Apply the knowledge record to a context, given some quality and clock")     

    // clears the value to a 0 integer
    .def ("clear_value", &madara::knowledge::KnowledgeRecord::clear_value,
      "Sets the value to 0 and type to integer")

    // decrements an index of an array
    .def ("dec_index", &madara::knowledge::KnowledgeRecord::dec_index,
      "Decrements an array element at a particular index")

    // returns true if record exists
    .def ("exists", &madara::knowledge::KnowledgeRecord::exists,
      "Returns whether the knowledge has been set/modified/created")

    // fragments the record
    .def ("fragment", &madara::knowledge::KnowledgeRecord::fragment,
     "Fragments the record into components")
 
    // gets the double precision
    .def ("get_precision", &madara::knowledge::KnowledgeRecord::get_precision,
      "Gets the double precision used in to_string")

    // increments an index of an array
    .def ("inc_index", &madara::knowledge::KnowledgeRecord::inc_index,
      "Increments an array element at a particular index")        

    // checks if record is a binary file type
    .def ("is_binary_file_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_binary_file_type),
      "Checks if the record is a binary file type")

    // checks if record is a double type
    .def ("is_double_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_double_type),
      "Checks if the record is a double type")     

    // checks if the record is false
    .def ("is_false", &madara::knowledge::KnowledgeRecord::is_false,
      "Checks if the record is false")

    // checks if record is a file type
    .def ("is_file_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_file_type),
      "Checks if the record is a file type")

    // checks if the record is an image type
    .def ("is_image_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_image_type),
      "Checks if the record is an image type")

    // checks if the record is an integer type
    .def ("is_integer_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_integer_type),
      "Checks if the record is an integer type")

    // checks if the record is a string type
    .def ("is_string_type",
      static_cast<bool (madara::knowledge::KnowledgeRecord::*)(void) const> (
      &madara::knowledge::KnowledgeRecord::is_string_type),
      "Checks if the record is a string type") 

    // checks if the record is true
    .def ("is_true", &madara::knowledge::KnowledgeRecord::is_true,
      "Checks if the record is true")         

    // sets the contents of the record as a file
    .def ("read_file", &madara::knowledge::KnowledgeRecord::read_file,
      m_read_file_1_of_2 (
      args("filename", "read_as_type"),
        "Reads the contents of a file into the record. Type "
        "is an optional field that can force a type for reading into."))

    // reset the record to UNCREATED
    .def ("reset_value", &madara::knowledge::KnowledgeRecord::reset_value,
      "Resets the record to an UNCREATED status (faster than clear_value)")        

    // resize an array
    .def ("resize", &madara::knowledge::KnowledgeRecord::resize,
      "Resizes an array to a new size")

    // retrieves an index of an array
    .def ("retrieve_index", &madara::knowledge::KnowledgeRecord::retrieve_index,
      "Retrieves an array element at a particular index")

    // sets a knowledge record to a double
    .def ("set",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      double)
      > (&madara::knowledge::KnowledgeRecord::set_value),
      "Sets the value to a double")

    // sets a knowledge record to an array of doubles
    .def ("set",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      const std::vector <double> & )
     > (&madara::knowledge::KnowledgeRecord::set_value),
     "Sets the value to an array of doubles")

    // sets a knowledge record to an integer
    .def ("set",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
       int )
      > (&madara::knowledge::KnowledgeRecord::set_value),
      "Sets the value to an integer")

    // sets a knowledge record to an array of integer
    .def ("set",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      const std::vector <madara::knowledge::KnowledgeRecord::Integer> & )
      > (&madara::knowledge::KnowledgeRecord::set_value),
      "Sets the value to an array of integers")

    // sets a knowledge record to a string
    .def ("set",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      const std::string & )
      > (&madara::knowledge::KnowledgeRecord::set_value),
      "Sets the value to a string")

    // sets a knowledge record to an Any
    .def ("set",
         +[](KnowledgeRecord &rec, const Any &any) {
          rec.emplace_any(any);
         },
      "Sets the value to an Any")     

    // sets the contents of the record to a file
    .def ("set_file", 
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      const unsigned char * new_value, size_t size )
      > (&madara::knowledge::KnowledgeRecord::set_file),
      "Sets the value to a file's contents")    

    // sets fixed precision
    .def ("set_fixed", &madara::knowledge::KnowledgeRecord::set_fixed,
      "Set the output format for doubles to std::fixed for madara logging")

    // sets an array index to an integer
    .def ("set_index",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      size_t,
      madara::knowledge::KnowledgeRecord::Integer)
      > (&madara::knowledge::KnowledgeRecord::set_index),
      "Sets an array index to an integer")

    // sets an array index to a double
    .def ("set_index",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      size_t,
      double)
      > (&madara::knowledge::KnowledgeRecord::set_index),
      "Sets an array index to a double (and converts the array to double)")      

    // sets the contents of the record to a jpeg
    .def ("set_jpeg",
      static_cast<
      void (madara::knowledge::KnowledgeRecord::*)(
      const unsigned char * new_value, size_t size )
      > (&madara::knowledge::KnowledgeRecord::set_jpeg),
      "Sets the value to a jpeg")       

    // sets the double precision
    .def ("set_precision", &madara::knowledge::KnowledgeRecord::set_precision,
      "Sets the double precision, generally for to_string")

    // sets scientific precision
    .def ("set_scientific", &madara::knowledge::KnowledgeRecord::set_scientific,
      "Sets the output format for doubles to std::scientific")

    // sets the contents of the record to a jpeg
    .def ("size", &madara::knowledge::KnowledgeRecord::size,
      "Returns the size of the value")

    // show record information
    .def ("status", &madara::knowledge::KnowledgeRecord::status,
      "Returns the status of the record")

    // convert to a double
    .def ("to_double", &madara::knowledge::KnowledgeRecord::to_double,
      "Converts the record to a double")

    // convert to a std::vector of doubles
    .def ("to_doubles", &madara::knowledge::KnowledgeRecord::to_doubles,
      "Converts the record to an array of doubles")

    // save value to a file
    .def ("to_file", &madara::knowledge::KnowledgeRecord::to_file,
      "Saves the value of the record to a file")     

    // convert to an integer
    .def ("to_integer", &madara::knowledge::KnowledgeRecord::to_integer,
     "Converts the record to an integer")

    // convert to a std::vector of integers
    .def ("to_integers", &madara::knowledge::KnowledgeRecord::to_integers,
      "Converts the record to an array of integers")     

    // convert to a string
    .def ("to_string", &madara::knowledge::KnowledgeRecord::to_string,
      m_to_string_0_of_1 (
      args ("delimiter"),
      "Converts the record to a string"))

    // gets the type of the record
    .def ("type", &madara::knowledge::KnowledgeRecord::type,
      "Returns the value type")

    // overloaded operators
    .def (self < self)
    .def (self <= self)
    .def (self == double ())
    .def (self == std::string ())
    .def (self == self)
    .def (self != self)
    .def (self > self)
    .def (self >= self)
    .def (!self)
    .def (-self)
    .def (self += self)
    .def (self -= self)
    .def (self *= self)
    .def (self /= self)
    .def (self %= self)
    .def (self * self)
    .def (self / self)
    .def (self % self)
    .def (self + self)
    .def (self - self)

     // class members
    .def_readwrite("clock",
      &madara::knowledge::KnowledgeRecord::clock,
      "last modification lamport clock")

    .def_readwrite("quality",
      &madara::knowledge::KnowledgeRecord::quality,
      "write priority for any local updates")
    
    .def_readwrite("toi",
      &madara::knowledge::KnowledgeRecord::toi,
      "last modification system clock time (time of insertion)")
  
    .def_readwrite("write_quality",
      &madara::knowledge::KnowledgeRecord::write_quality,
      "priority of the update")
      
    // because of the templated nature of KnowledgeRecord::operator=, I could not
    // figure this out
    // .def ("operator=", 
    // static_cast<madara::knowledge::KnowledgeRecord (
    // madara::knowledge::KnowledgeRecord::*)(
    // int &&)> (
    // &madara::knowledge::KnowledgeRecord::operator=),
    // "Assigns an int to the record",
    // return_value_policy<reference_existing_object> ())

    .def ("operator==",
    static_cast<bool (
    madara::knowledge::KnowledgeRecord::*)(
    const madara::knowledge::KnowledgeRecord &) const> (
    &madara::knowledge::KnowledgeRecord::operator==),
    "Compares two records for equality")

    .def ("operator<=",
    static_cast<bool (
    madara::knowledge::KnowledgeRecord::*)(
    const madara::knowledge::KnowledgeRecord &) const> (
    &madara::knowledge::KnowledgeRecord::operator<=),
    "Compares two records with less than or equal to")

    .def ("operator>=",
    static_cast<bool (
    madara::knowledge::KnowledgeRecord::*)(
    const madara::knowledge::KnowledgeRecord &) const> (
    &madara::knowledge::KnowledgeRecord::operator>=),
    "Compares two records with greater than or equal to")

    .def ("operator>",
    static_cast<bool (
    madara::knowledge::KnowledgeRecord::*)(
    const madara::knowledge::KnowledgeRecord &) const> (
    &madara::knowledge::KnowledgeRecord::operator>),
    "Compares two records with greater than")

    .def ("operator<",
    static_cast<bool (
    madara::knowledge::KnowledgeRecord::*)(
    const madara::knowledge::KnowledgeRecord &) const> (
    &madara::knowledge::KnowledgeRecord::operator<),
    "Compares two records with less than")

    .def ("operator++",
    &madara::knowledge::KnowledgeRecord::operator++,
    "Adds one to the record",
    return_value_policy<reference_existing_object> ())

    .def ("operator--",
    &madara::knowledge::KnowledgeRecord::operator--,
    "Subtracts one from the record",
    return_value_policy<reference_existing_object> ())

    .def ("to_any", MADARA_MEMB(Any, KnowledgeRecord, to_any, () const),
        "Convert this record to an Any")

    .staticmethod("get_precision")
    .staticmethod("set_fixed")
    .staticmethod("set_precision")
    .staticmethod("set_scientific")

    ; // end of KnowledgeRecord


     // the types of values supported in KnowledgeRecord
     enum_<madara::knowledge::KnowledgeRecord::ValueTypes> ("ValueTypes")
     .value ("EMPTY",
     madara::knowledge::KnowledgeRecord::EMPTY)
     .value ("INTEGER",
     madara::knowledge::KnowledgeRecord::INTEGER)
     .value ("STRING",
     madara::knowledge::KnowledgeRecord::STRING)
     .value ("DOUBLE",
     madara::knowledge::KnowledgeRecord::DOUBLE)
     .value ("FLOAT",
     madara::knowledge::KnowledgeRecord::FLOAT)
     .value ("UNKNOWN_FILE_TYPE",
     madara::knowledge::KnowledgeRecord::UNKNOWN_FILE_TYPE)
     .value ("XML",
     madara::knowledge::KnowledgeRecord::XML)
     .value ("TEXT_FILE",
     madara::knowledge::KnowledgeRecord::TEXT_FILE)
     .value ("DOUBLE_ARRAY",
     madara::knowledge::KnowledgeRecord::DOUBLE_ARRAY)
     .value ("INTEGER_ARRAY",
     madara::knowledge::KnowledgeRecord::INTEGER_ARRAY)
     .value ("IMAGE_JPEG",
     madara::knowledge::KnowledgeRecord::IMAGE_JPEG)
     .value ("ALL_ARRAYS",
     madara::knowledge::KnowledgeRecord::ALL_ARRAYS)
     .value ("ALL_INTEGERS",
     madara::knowledge::KnowledgeRecord::ALL_INTEGERS)
     .value ("ALL_DOUBLES",
     madara::knowledge::KnowledgeRecord::ALL_DOUBLES)
     .value ("ALL_PRIMITIVE_TYPES",
     madara::knowledge::KnowledgeRecord::ALL_PRIMITIVE_TYPES)
     .value ("ALL_FILE_TYPES",
     madara::knowledge::KnowledgeRecord::ALL_FILE_TYPES)
     .value ("ALL_IMAGES",
     madara::knowledge::KnowledgeRecord::ALL_IMAGES)
     .value ("ALL_TEXT_FORMATS",
     madara::knowledge::KnowledgeRecord::ALL_TEXT_FORMATS)
     .value ("ALL_TYPES",
     madara::knowledge::KnowledgeRecord::ALL_TYPES)
     .value ("ALL_CLEARABLES",
     madara::knowledge::KnowledgeRecord::ALL_CLEARABLES)
     
   ; // end of ValueTypes

   // Boost.python does not appear to support this type of function
   //.def ("to_unmanaged_buffer", &madara::knowledge::KnowledgeRecord::to_unmanaged_buffer)


  /********************************************************
    * KnowledgeReferenceSettings definitions
    ********************************************************/
 
  class_<madara::knowledge::KnowledgeReferenceSettings> (
      "KnowledgeReferenceSettings",
      "Settings for referencing knowledge records", init<> ())
        
    .def (init <bool> ())
    .def (init <
      const madara::knowledge::KnowledgeReferenceSettings &> ())

    // define readwrite variables within the class
    .def_readwrite ("expand_variables",
      &madara::knowledge::KnowledgeReferenceSettings::expand_variables,
      "Always attempt to expand variable names")
    .def_readwrite ("never_exit",
      &madara::knowledge::KnowledgeReferenceSettings::never_exit,
      "Do not exit, even if in invalid states")

  ; // end class KnowledgeReferenceSettings
          
  /********************************************************
    * KnowledgeUpdateSettings definitions
    ********************************************************/
 
  class_<madara::knowledge::KnowledgeUpdateSettings,
          bases<madara::knowledge::KnowledgeReferenceSettings> > (
            "KnowledgeUpdateSettings",
            "Settings for updating knowledge records", init <> ())
    
    // other constructors here
    .def (init <
      const madara::knowledge::KnowledgeUpdateSettings &> ())

    // define readwrite variables within the class
    .def_readwrite ("treat_globals_as_locals",
      	&madara::knowledge::KnowledgeUpdateSettings::treat_globals_as_locals,
        "Treat global variable changes as if they were local variables")
    .def_readwrite ("signal_changes",
      	&madara::knowledge::KnowledgeUpdateSettings::signal_changes,
        "Always signal on mutation (useful for multi-threaded applications)")
    .def_readwrite ("always_overwrite",
      	&madara::knowledge::KnowledgeUpdateSettings::always_overwrite,
        "Always overwrite records, regardless of quality or consistency checks")
    .def_readwrite ("track_local_changes",
      	&madara::knowledge::KnowledgeUpdateSettings::track_local_changes,
        "Track local changes for checkpointing purposes")
    .def_readwrite ("clock_increment",
      	&madara::knowledge::KnowledgeUpdateSettings::clock_increment,
        "Integer increment for updates to Lamport clocks")
    .def_readwrite ("treat_locals_as_globals",
      	&madara::knowledge::KnowledgeUpdateSettings::treat_locals_as_globals,
        "Toggle whether updates to local variables are treated as"
        "global variables that should be sent over the transport. It"
        "should be stressed that this is dangerous and should only"
        "be used for debugging. If you toggle this to true, all local"
        "variables will be sent over the network where they will"
        "overwrite local variables in remote systems, unless the"
        "remote system filters out the local variable changes with"
        "an on-receive filter")
    .def_readwrite ("stream_changes",
      	&madara::knowledge::KnowledgeUpdateSettings::stream_changes,
        "Toggle for streaming support. If this is true, all changes"
        "will be streamed to the attached streamer, if any")
  ; // end class KnowledgeUpdateSettings
          
  /********************************************************
    * EvalSettings definitions
    ********************************************************/
 
  class_<madara::knowledge::EvalSettings,
          bases<madara::knowledge::KnowledgeUpdateSettings> > (
            "EvalSettings",
            "Settings for evaluate calls", init <> ())
    
    // other constructors here
    .def (init <
      const madara::knowledge::EvalSettings &> ())
    
    // define readwrite variables within the class
    .def_readwrite ("delay_sending_modifieds",
      	&madara::knowledge::EvalSettings::delay_sending_modifieds,
        "Do not send modifications after an evaluation")
    .def_readwrite ("pre_print_statement",
      	&madara::knowledge::EvalSettings::pre_print_statement,
        "Statement to atomically expand and print before an evaluate")
    .def_readwrite ("post_print_statement",
      	&madara::knowledge::EvalSettings::post_print_statement,
        "Statement to atomically expand and print after an evaluate")
    .def_readwrite ("send_list",
      	&madara::knowledge::EvalSettings::send_list,
        "List of variables that are allowed to be sent, if changed")

  ; // end class EvalSettings
          
  /********************************************************
    * WaitSettings definitions
    ********************************************************/
 
  class_<madara::knowledge::WaitSettings,
          bases<madara::knowledge::EvalSettings> > ("WaitSettings",
          "Settings for wait calls", init <> ())
    
    // other constructors here
    .def (init <
      const madara::knowledge::WaitSettings &> ())
        
    // define readwrite variables within the class
    .def_readwrite ("poll_frequency",
      	&madara::knowledge::WaitSettings::poll_frequency,
        "Frequency, in seconds, to evaluate the expression")
    .def_readwrite ("max_wait_time",
      	&madara::knowledge::WaitSettings::max_wait_time,
        "Maximum amount of time to wait for truth")
  ; // end class WaitSettings

  /********************************************************
    * Variables definitions
    ********************************************************/
    
  class_<madara::knowledge::Variables> ("Variables", init <> ())

    // add list of current modified list
    .def ("add_modifieds",
      &madara::knowledge::KnowledgeBase::add_modifieds,
      "Adds a list of VariableReferences to the current modified list")        

    // modifies all global variables
    .def ("apply_modified",
      &madara::knowledge::Variables::apply_modified,
      "Applies modified to all global variables")

    // compile prototype
    .def ("compile",
      &madara::knowledge::Variables::compile,
      "Compiles a KaRL expression into an expression tree. Always do this"
      "before calling evaluate because it puts the expression into an"
      "optimized format. Best practice is to save the CompiledExpression"
      "in a global variable or in some kind of persistent store. Pair with"
      "expand_statement if you know that variable expansion is used but"
      "the variable values that are expanded never change (e.g. an id that"
      "is set through the command line and thus stays the same after it is"
      "initially set")     

    // decrements the value of a variable
    .def( "dec",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const std::string &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::dec),
      "Atomically decrements the value of the variable")        

    // decrements the value of a variable
    .def( "dec",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::dec),
      "Atomically decrements the value of the variable") 

    // evaluate an expression
    .def( "evaluate",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const std::string &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::evaluate),
        m_eval_1_of_2 (
        args("expression", "settings"),
        "Evaluates an expression"))

    // evaluate an expression
    .def( "evaluate",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          madara::knowledge::CompiledExpression &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::evaluate),
        m_eval_1_of_2 (
        args("expression", "settings"),
        "Evaluates an expression"))      

    // check to see if variable exists
    .def( "exists",
      static_cast<
        bool (madara::knowledge::Variables::*)(
          const std::string &, 
          const madara::knowledge::KnowledgeReferenceSettings &) const 
      > (&madara::knowledge::Variables::exists),
      "Checks if a knowledge location exists in the context") 
        
    // check to see if variable exists
    .def( "exists",
      static_cast<
        bool (madara::knowledge::Variables::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeReferenceSettings &) const
      > (&madara::knowledge::Variables::exists),
      "Checks if a knowledge variable exists in the context") 

    // prints all knowledge variables
    .def ("expand_statement",
      &madara::knowledge::Variables::expand_statement,
      "Expand a statement") 

    // get a knowledge record
    .def( "get",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const std::string &,
          const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::Variables::get),
      m_get_1_of_2 (
        args("key", "settings"),
        "Retrieves the value of a variable"))

    // get a knowledge record
    .def( "get",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::Variables::get),
      m_get_1_of_2 (
        args("variable", "settings"),
        "Retrieves the value of a variable"))

    // converts to a string
    .def ("get_ref",
      &madara::knowledge::Variables::get_ref,
      "Retrieves the value of a variable")

    // get matches in an iterable form
    .def ("get_matches",
      &madara::knowledge::Variables::get_matches,
      "Creates an iteration of VariableReferences to all keys matching"
      "the prefix and suffix")

    // increments the value of a variable
    .def( "inc",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const std::string &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::inc),
      "Atomically increments the value of the variable") 

    // increments the value of a variable
    .def( "inc",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const madara::knowledge::VariableReference &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::inc),
      "Atomically increments the value of the variable")  

    // Loads the context from a file
    .def ("load_context",
      &madara::knowledge::Variables::load_context,
      "Loads the context from a file")   

    // print statement
    .def( "print",
      static_cast<
        void (madara::knowledge::Variables::*)(
          unsigned int) const
      > (&madara::knowledge::Variables::print),
      "Prints all knowledge in the context") 
        
    // print statement
    .def( "print",
      static_cast<
        void (madara::knowledge::Variables::*)(
          const std::string &, unsigned int) const
      > (&madara::knowledge::Variables::print),
      "Prints a statement")       

    // get a knowledge record at an index
    .def( "retrieve_index",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const std::string &,
            size_t,
            const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::Variables::retrieve_index),
      m_retrieve_index_2_of_3 (
        args("key", "index", "settings"),
        "Retrieves a knowledge record from an index"))

    // get a knowledge record at an index
    .def( "retrieve_index",
      static_cast<
        madara::knowledge::KnowledgeRecord (madara::knowledge::Variables::*)(
          const VariableReference &,
            size_t,
            const madara::knowledge::KnowledgeReferenceSettings &)
      > (&madara::knowledge::Variables::retrieve_index),
      m_retrieve_index_2_of_3 (
        args("key", "index", "settings"),
        "Retrieves a knowledge record from an index")) 

    // save context file to karl
    .def ("save_as_karl",
      &madara::knowledge::Variables::save_as_karl,
      "Saves the context to a file as karl assignments, rather than binary")

    // save a checkpoint to a file
    .def ("save_checkpoint",
      &madara::knowledge::Variables::save_checkpoint,
      "Saves a checkpoint of a list of changes to a file")        

    // save context to a file
    .def ("save_context",
      &madara::knowledge::Variables::save_context,
      "Saves the context to a file")

    // sets a knowledge record to a double
    .def( "set",
      static_cast<
        int (madara::knowledge::Variables::*)(
          const std::string &,
          double,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::set),
      "sets a knowledge record to a double") 
        
    // sets a knowledge record to an array of doubles
    .def( "set",
      static_cast<
        int (madara::knowledge::Variables::*)(
          const std::string &,
          const std::vector <double> &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::set),
      "sets a knowledge record to an array of doubles") 
        
    // sets a knowledge record to an integer
    .def( "set",
      static_cast<
        int (madara::knowledge::Variables::*)(
          const std::string &,
          madara::knowledge::KnowledgeRecord::Integer,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::set),
      "sets a knowledge record to a integer") 
        
    // sets a knowledge record to an array of integer
    .def( "set",
      static_cast<
        int (madara::knowledge::Variables::*)(
          const std::string &,
          const std::vector <madara::knowledge::KnowledgeRecord::Integer> &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::set),
      "sets a knowledge record to an array of integers") 
        
    // sets a knowledge record to a string
    .def( "set",
      static_cast<
        int (madara::knowledge::Variables::*)(
          const std::string &,
          const std::string &,
          const madara::knowledge::KnowledgeUpdateSettings &)
      > (&madara::knowledge::Variables::set),
      "sets a knowledge record to a string")  

    // fill a variable map with knowledge records
    .def ("to_map",
      &madara::knowledge::Variables::to_map,
      "Fills a variable map with Knowledge Records that match an expression."
      "At the moment, this expression must be of the form 'subject'")           

    // converts to a string
    .def ("to_string",
      &madara::knowledge::Variables::to_string,
      "Converts to string")  

    // fill a vector with knowledge records
    .def ("to_vector",
      &madara::knowledge::Variables::to_vector,
      "Fills a vector with Knowledge Records that begin with a common subject"
      "and have a finite range of integer values")

    // write a file to a location
    .def ("write_file",
      &madara::knowledge::Variables::write_file,
      "Write a file from the context to a specified location")

  ;
    
  /********************************************************
    * FunctionArguments definitions
    ********************************************************/
 
  class_ <std::vector <madara::knowledge::FunctionArguments> > (
        "FunctionArguments", "List of arguments to a function")
  ;
    
 

  class_ <std::vector <madara::knowledge::KnowledgeRecord> > ("KnowledgeRecordVector")
    .def(vector_indexing_suite<std::vector<madara::knowledge::KnowledgeRecord> >());
  
  class_ <std::map <std::string, madara::knowledge::KnowledgeRecord> > (
    "KnowledgeRecordMap")
    .def(map_indexing_suite<std::map <
      std::string, madara::knowledge::KnowledgeRecord> >());

  // define complex classes in the madara.knowledge module space
  define_knowledge_base ();

  // define madara.knowledge.containers
  define_knowledge_containers ();
}

#endif // _MADARA_PYTHON_PORT_MADARA_KNOWLEGE_CPP_

