// copy-relocs.h -- handle COPY relocations for gold   -*- C++ -*-

// Copyright 2006, 2007, 2008 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#ifndef GOLD_COPY_RELOCS_H
#define GOLD_COPY_RELOCS_H

#include "elfcpp.h"
#include "reloc-types.h"
#include "output.h"

namespace gold
{

// This class is used to manage COPY relocations.  We try to avoid
// them when possible.  A COPY relocation may be required when an
// executable refers to a variable defined in a shared library.  COPY
// relocations are problematic because they tie the executable to the
// exact size of the variable in the shared library.  We can avoid
// them if all the references to the variable are in a writeable
// section.  In that case we can simply use dynamic relocations.
// However, when scanning relocs, we don't know when we see the
// relocation whether we will be forced to use a COPY relocation or
// not.  So we have to save the relocation during the reloc scanning,
// and then emit it as a dynamic relocation if necessary.  This class
// implements that.  It is used by the target specific code.

// The template parameter SH_TYPE is the type of the reloc section to
// be used for COPY relocs: elfcpp::SHT_REL or elfcpp::SHT_RELA.

template<int sh_type, int size, bool big_endian>
class Copy_relocs
{
 private:
  typedef typename Reloc_types<sh_type, size, big_endian>::Reloc Reloc;

 public:
  Copy_relocs(unsigned int copy_reloc_type)
    : copy_reloc_type_(copy_reloc_type), dynbss_(NULL), entries_()
  { }

  // This is called while scanning relocs if we see a relocation
  // against a symbol which may force us to generate a COPY reloc.
  // SYM is the symbol.  OBJECT is the object whose relocs we are
  // scanning.  The relocation is being applied to section SHNDX in
  // OBJECT.  OUTPUT_SECTION is the output section where section SHNDX
  // will wind up.  REL is the reloc itself.  The Output_data_reloc
  // section is where the dynamic relocs are put.
  void
  copy_reloc(Symbol_table*, Layout*, Sized_symbol<size>* sym,
             Sized_relobj_file<size, big_endian>* object,
	     unsigned int shndx, Output_section* output_section,
	     const Reloc& rel,
	     Output_data_reloc<sh_type, true, size, big_endian>*);

  // Return whether there are any saved relocations.
  bool
  any_saved_relocs() const
  { return !this->entries_.empty(); }

  // Emit any saved relocations which turn out to be needed.  This is
  // called after all the relocs have been scanned.
  void
  emit(Output_data_reloc<sh_type, true, size, big_endian>*);

 private:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Addend;

  // This POD class holds the relocations we are saving.  We will emit
  // these relocations if it turns out that the symbol does not
  // require a COPY relocation.
  class Copy_reloc_entry
  {
   public:
    Copy_reloc_entry(Symbol* sym, unsigned int reloc_type,
		     Sized_relobj_file<size, big_endian>* relobj,
                     unsigned int shndx,
		     Output_section* output_section,
		     Address address, Addend addend)
      : sym_(sym), reloc_type_(reloc_type), relobj_(relobj),
	shndx_(shndx), output_section_(output_section),
	address_(address), addend_(addend)
    { }

    // Emit this reloc if appropriate.  This is called after we have
    // scanned all the relocations, so we know whether we emitted a
    // COPY relocation for SYM_.
    void
    emit(Output_data_reloc<sh_type, true, size, big_endian>*);

   private:
    Symbol* sym_;
    unsigned int reloc_type_;
    Sized_relobj_file<size, big_endian>* relobj_;
    unsigned int shndx_;
    Output_section* output_section_;
    Address address_;
    Addend addend_;
  };

  // A list of relocs to be saved.
  typedef std::vector<Copy_reloc_entry> Copy_reloc_entries;

  // Return whether we need a COPY reloc.
  bool
  need_copy_reloc(Sized_symbol<size>* gsym,
                  Sized_relobj_file<size, big_endian>* object,
		  unsigned int shndx) const;

  // Emit a COPY reloc.
  void
  emit_copy_reloc(Symbol_table*, Layout*, Sized_symbol<size>*,
		  Output_data_reloc<sh_type, true, size, big_endian>*);

  // Add a COPY reloc to the dynamic reloc section.
  void
  add_copy_reloc(Symbol*, section_size_type,
		 Output_data_reloc<sh_type, true, size, big_endian>*);

  // Save a reloc against SYM for possible emission later.
  void
  save(Symbol*, Sized_relobj_file<size, big_endian>*, unsigned int shndx,
       Output_section*, const Reloc& rel);

  // The target specific relocation type of the COPY relocation.
  const unsigned int copy_reloc_type_;
  // The dynamic BSS data which goes into the .bss section.  This is
  // where variables which require COPY relocations are placed.
  Output_data_space* dynbss_;
  // The list of relocs we are saving.
  Copy_reloc_entries entries_;
};

} // End namespace gold.

#endif // !defined(GOLD_COPY_RELOCS_H)
