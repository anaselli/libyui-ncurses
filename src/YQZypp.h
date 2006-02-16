/*-----------------------------------------------------------*- c++ -*-\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:		YQZypp.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YQZypp_h
#define YQZypp_h


#include <zypp/ui/Status.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ResObject.h>
#include <zypp/Package.h>
#include <zypp/Selection.h>
#include <zypp/Pattern.h>
#include <zypp/Patch.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ResPoolProxy.h>


using zypp::ui::S_Protected;           
using zypp::ui::S_Taboo;               
using zypp::ui::S_Del;                 
using zypp::ui::S_Update;              
using zypp::ui::S_Install;             
using zypp::ui::S_AutoDel;             
using zypp::ui::S_AutoUpdate;          
using zypp::ui::S_AutoInstall;         
using zypp::ui::S_KeepInstalled;       
using zypp::ui::S_NoInst;              


//
// Typedefs to make those nested namespaces human-readable
//

typedef zypp::ui::Status			ZyppStatus;
typedef zypp::ui::Selectable::Ptr		ZyppSel;
typedef zypp::Selection::constPtr		ZyppSelection;
typedef zypp::Pattern::constPtr			ZyppPattern;
typedef zypp::Patch::constPtr			ZyppPatch;

#if 1
typedef zypp::ResObject		ZyppObj_;
typedef zypp::Package		ZyppPkg_;
#else
class ResObjectCompat;
class ResObjectMixin
{
public:
    typedef ResObjectCompat Self; // really
    typedef zypp::ResTraits<Self> TraitsType;
    typedef TraitsType::PtrType Ptr;
    typedef TraitsType::constPtrType constPtr;

    // missing methods
    bool hasSelectable () const { return false; }
    ZyppSel getSelectable () const { return 0; }
    bool hasCandidateObj () const { return false; }
    Ptr getCandidateObj () const { return 0; }
    bool hasInstalledObj () const { return false; }
    Ptr getInstalledObj () const { return 0; }
};
// fooling the compiler until I know how to fix
DEFINE_PTR_TYPE(ResObjectCompat);
class ResObjectCompat : public zypp::ResObject, public ResObjectMixin
{
public:
    typedef ResObjectCompat Self;
    typedef zypp::ResTraits<Self> TraitsType;
    typedef TraitsType::PtrType Ptr;
    typedef TraitsType::constPtrType constPtr;
};
typedef ResObjectCompat ZyppObj_;

// fooling the compiler until I know how to fix
DEFINE_PTR_TYPE(PackageCompat);
class PackageCompat : public zypp::Package, public ResObjectMixin
{
public:
    typedef PackageCompat Self;
    typedef zypp::ResTraits<Self> TraitsType;
    typedef TraitsType::PtrType Ptr;
    typedef TraitsType::constPtrType constPtr;

    // missing methods
    bool hasLicenseToConfirm () const { return true; }
    void markLicenseConfirmed () const {}
};
typedef PackageCompat ZyppPkg_;
#endif
typedef ZyppObj_::constPtr ZyppObj;
typedef ZyppPkg_::constPtr ZyppPkg;

typedef zypp::ResPoolProxy			ZyppPool;
typedef zypp::ResPoolProxy::const_iterator	ZyppPoolIterator;


inline ZyppPool		zyppPool()		{ return zypp::getZYpp()->poolProxy();	}

template<class T> ZyppPoolIterator zyppBegin()	{ return zyppPool().byKindBegin<T>();	}
template<class T> ZyppPoolIterator zyppEnd()	{ return zyppPool().byKindEnd<T>();	}

inline ZyppPoolIterator zyppPkgBegin()		{ return zyppBegin<zypp::Package>();		}
inline ZyppPoolIterator zyppPkgEnd()		{ return zyppEnd<zypp::Package>();		}

inline ZyppPoolIterator zyppSelectionsBegin()	{ return zyppBegin<zypp::Selection>();	}
inline ZyppPoolIterator zyppSelectionsEnd()	{ return zyppEnd<zypp::Selection>();	}

inline ZyppPoolIterator zyppPatternsBegin()	{ return zyppBegin<zypp::Pattern>();	}
inline ZyppPoolIterator zyppPatternsEnd()	{ return zyppEnd<zypp::Pattern>();	}

inline ZyppPoolIterator zyppPatchesBegin()	{ return zyppBegin<zypp::Patch>();	}
inline ZyppPoolIterator zyppPatchesEnd()	{ return zyppEnd<zypp::Patch>();		}


inline ZyppPkg		tryCastToZyppPkg( ZyppObj zyppObj )
{
    return zypp::dynamic_pointer_cast<const ZyppPkg_>( zyppObj );
}

inline ZyppSelection	tryCastToZyppSelection( ZyppObj zyppObj )
{
    return zypp::dynamic_pointer_cast<const zypp::Selection>( zyppObj );
}

inline ZyppPattern 	tryCastToZyppPattern( ZyppObj zyppObj )
{
    return zypp::dynamic_pointer_cast<const zypp::Pattern>( zyppObj );
}

inline ZyppPatch	tryCastToZyppPatch( ZyppObj zyppObj )
{
    return zypp::dynamic_pointer_cast<const zypp::Patch>( zyppObj );
}



#endif // YQZypp_h