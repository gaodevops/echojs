/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99 ft=cpp:
 */

#include "ejs-llvm.h"
#include "ejs-object.h"
#include "ejs-value.h"
#include "ejs-array.h"
#include "ejs-function.h"
#include "ejs-string.h"
#include "ejs-proxy.h"
#include "ejs-symbol.h"
#include "ejs-error.h"
#include "function.h"
#include "functiontype.h"
#include "type.h"
#include "value.h"

namespace ejsllvm {

    typedef struct {
        /* object header */
        EJSObject obj;

        /* type specific data */
        llvm::Function *llvm_fun;
    } Function;

    static EJSSpecOps _ejs_Function_specops EJSVAL_ALIGNMENT;
    static ejsval _ejs_Function_prototype EJSVAL_ALIGNMENT;
    static ejsval _ejs_Function;

    static EJSObject* Function_allocate()
    {
        return (EJSObject*)_ejs_gc_new(Function);
    }

    static ejsval
    Function_create (ejsval env, ejsval _this, int argc, ejsval *args)
    {
        ejsval F = _this;
        if (!EJSVAL_IS_CONSTRUCTOR(F)) 
            _ejs_throw_nativeerror_utf8 (EJS_TYPE_ERROR, "'this' in Function[Symbol.create] is not a constructor");
        EJSObject* F_ = EJSVAL_TO_OBJECT(F);
        // 2. Let obj be the result of calling OrdinaryCreateFromConstructor(F, "%DatePrototype%", ([[DateData]]) ). 
        ejsval proto = OP(F_,Get)(F, _ejs_atom_prototype, F);
        if (EJSVAL_IS_UNDEFINED(proto))
            proto = _ejs_Function_prototype;

        EJSObject* obj = (EJSObject*)_ejs_gc_new (Function);
        _ejs_init_object (obj, proto, &_ejs_Function_specops);
        return OBJECT_TO_EJSVAL(obj);
    }

    static ejsval
    Function_impl (ejsval env, ejsval _this, int argc, ejsval *args)
    {
        EJS_NOT_IMPLEMENTED();
    }

    ejsval
    Function_new(llvm::Function* llvm_fun)
    {
        ejsval result = _ejs_object_new (_ejs_Function_prototype, &_ejs_Function_specops);
        ((Function*)EJSVAL_TO_OBJECT(result))->llvm_fun = llvm_fun;
        return result;
    }

    ejsval
    Function_prototype_toString(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        std::string str;
        llvm::raw_string_ostream str_ostream(str);
        ((Function*)EJSVAL_TO_OBJECT(_this))->llvm_fun->print(str_ostream);

        return _ejs_string_new_utf8(trim(str_ostream.str()).c_str());
    }

    ejsval
    Function_prototype_dump(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        ((Function*)EJSVAL_TO_OBJECT(_this))->llvm_fun->dump();
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setOnlyReadsMemory(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->setOnlyReadsMemory();
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setDoesNotAccessMemory(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->setDoesNotAccessMemory();
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setDoesNotThrow(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->setDoesNotThrow();
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setGC(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        REQ_UTF8_ARG(0, name);
        fun->llvm_fun->setGC(name.c_str());
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setExternalLinkage(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->setLinkage (llvm::Function::ExternalLinkage);
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setInternalLinkage(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->setLinkage (llvm::Function::InternalLinkage);
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_setStructRet(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        fun->llvm_fun->addAttribute(1 /* first arg */,
                                    llvm::Attribute::StructRet);
        return _ejs_undefined;
    }

    ejsval
    Function_prototype_hasStructRetAttr(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return fun->llvm_fun->hasStructRetAttr() ? _ejs_true : _ejs_false;
    }

    ejsval
    Function_prototype_get_args(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        unsigned size = fun->llvm_fun->arg_size();
        ejsval result = _ejs_array_new(0, EJS_FALSE);

        unsigned Idx = 0;
        for (llvm::Function::arg_iterator AI = fun->llvm_fun->arg_begin(); Idx != size;
             ++AI, ++Idx) {
            ejsval val = Value_new(AI);
            _ejs_array_push_dense (result, 1, &val);
        }
        return result;
    }

    ejsval
    Function_prototype_get_argSize(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return NUMBER_TO_EJSVAL (fun->llvm_fun->arg_size());
    }

    ejsval
    Function_prototype_get_returnType(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return Type_new (fun->llvm_fun->getReturnType());
    }

    ejsval
    Function_prototype_get_name(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        std::string fun_name = fun->llvm_fun->getName();
        return _ejs_string_new_utf8(fun_name.c_str());
    }

    ejsval
    Function_prototype_get_type(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return FunctionType_new (fun->llvm_fun->getFunctionType());
    }

    ejsval
    Function_prototype_get_doesNotThrow(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return BOOLEAN_TO_EJSVAL(fun->llvm_fun->doesNotThrow());
    }

    ejsval
    Function_prototype_get_doesNotAccessMemory(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return BOOLEAN_TO_EJSVAL(fun->llvm_fun->doesNotAccessMemory());
    }

    ejsval
    Function_prototype_get_onlyReadsMemory(ejsval env, ejsval _this, int argc, ejsval *args)
    {
        Function* fun = ((Function*)EJSVAL_TO_OBJECT(_this));
        return BOOLEAN_TO_EJSVAL(fun->llvm_fun->onlyReadsMemory());
    }

    llvm::Function*
    Function_GetLLVMObj(ejsval val)
    {
        if (EJSVAL_IS_NULL(val)) return NULL;
        return ((Function*)EJSVAL_TO_OBJECT(val))->llvm_fun;
    }

    void
    Function_init (ejsval exports)
    {
        _ejs_Function_specops = _ejs_Object_specops;
        _ejs_Function_specops.class_name = "LLVMFunction";
        _ejs_Function_specops.Allocate = Function_allocate;

        _ejs_gc_add_root (&_ejs_Function_prototype);
        _ejs_Function_prototype = _ejs_object_new(_ejs_Object_prototype, &_ejs_Function_specops);

        _ejs_Function = _ejs_function_new_utf8_with_proto (_ejs_null, "LLVMFunction", (EJSClosureFunc)Function_impl, _ejs_Function_prototype);

        _ejs_object_setprop_utf8 (exports,              "Function", _ejs_Function);

#define PROTO_METHOD(x) EJS_INSTALL_ATOM_FUNCTION(_ejs_Function_prototype, x, Function_prototype_##x)
#define PROTO_ACCESSOR(x) EJS_INSTALL_ATOM_GETTER(_ejs_Function_prototype, x, Function_prototype_get_##x)

        PROTO_ACCESSOR(args);
        PROTO_ACCESSOR(argSize);
        PROTO_ACCESSOR(name);
        PROTO_ACCESSOR(returnType);
        PROTO_ACCESSOR(type);
        PROTO_ACCESSOR(doesNotThrow);
        PROTO_ACCESSOR(onlyReadsMemory);
        PROTO_ACCESSOR(doesNotAccessMemory);

        PROTO_METHOD(dump);
        PROTO_METHOD(setOnlyReadsMemory);
        PROTO_METHOD(setDoesNotAccessMemory);
        PROTO_METHOD(setDoesNotThrow);
        PROTO_METHOD(setGC);
        PROTO_METHOD(setExternalLinkage);
        PROTO_METHOD(setInternalLinkage);
        PROTO_METHOD(toString);

        PROTO_METHOD(hasStructRetAttr);
        PROTO_METHOD(setStructRet);

#undef PROTO_METHOD
#undef PROTO_ACCESSOR

        EJS_INSTALL_SYMBOL_FUNCTION_FLAGS (_ejs_Function, create, Function_create, EJS_PROP_NOT_ENUMERABLE);
    }

};
