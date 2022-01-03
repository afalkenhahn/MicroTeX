#ifndef LATEX_ATOM_ROW_H
#define LATEX_ATOM_ROW_H

#include <bitset>

#include "utils/utils.h"
#include "atom/atom.h"
#include "box/box.h"
#include "unimath/uni_char.h"
#include "atom/atom_text.h"

namespace tex {

class AtomDecor;

class FixedCharAtom;

class CharSymbol;

class Env;

/**
 * A "composed atom": An atom that consists of child atoms that will be
 * displayed next to each other horizontally with glue between them.
 */
class Row {
public:
  /**
   * Sets the given "decor" containing the atom that comes just before the first
   * child atom of this "composed atom". This method will always be called by
   * another composed atom, so this composed atom will be a child of it
   * (nested). This is necessary to determine the glue to insert between the
   * first child atom of this nested composed atom and the atom that the "decor"
   * contains.
   *
   * @param decor
   *      the atom that comes just before this "composed atom"
   */
  virtual void setPreviousAtom(const sptr<AtomDecor>& decor) = 0;
};

/**
 * Used by RowAtom. The "textSymbol"-property and the type of an atom can changed
 * (according to the TeX-algorithms used). Or this atom can be replaced by a
 * ligature, (if it was a CharAtom). But atoms cannot be changed, otherwise
 * different boxes could be made from the same formula, and that is not
 * desired! This "atom decor" makes sure that changes to an atom (during the
 * createBox-method of a RowAtom) will be reset.
 */
class AtomDecor {
private:
  sptr<Atom> _atom;
  bool _textSymbol = false;

public:
  AtomType _type = AtomType::none;

  AtomDecor() = delete;

  explicit AtomDecor(const sptr<Atom>& atom) {
    _textSymbol = false;
    _atom = atom;
    _type = AtomType::none;
  }

  /** @return the changed type, or the old left type if it has not been changed */
  inline AtomType leftType() const {
    return (_type != AtomType::none ? _type : _atom->leftType());
  }

  /** @return the changed type, or the old right type if it has not been changed */
  inline AtomType rightType() const {
    return (_type != AtomType::none ? _type : _atom->rightType());
  }

  /** Test if this atom is a char-symbol. */
  bool isChar() const;

  /** Test if this char is in math mode. */
  bool isMathMode() const;

  /** This method will only be called if #isChar returns true. */
  Char getChar(Env& env) const;

  /** Changes this atom into the given "ligature atom". */
  void changeAtom(const sptr<FixedCharAtom>& atom);

  sptr<Box> createBox(Env& env);

  inline void markAsTextSymbol() {
    _textSymbol = true;
  }

  /** Test if this atom is a kern. */
  bool isKern() const;

  /** Only for row-elements, for nested rows */
  void setPreviousAtom(const sptr<AtomDecor>& prev);
};

/**
 * An atom representing a horizontal row of other atoms, to be separated by
 * glue. It's also responsible for inserting kerns and ligature.
 */
class RowAtom : public Atom, public Row {
private:
  // set of atom types that make a previous bin atom change to ord
  static std::bitset<16> _binSet;
  // set of atom types that can possibly need a kern or, together
  // with the previous atom, be replaced by a ligature
  static std::bitset<16> _ligKernSet;

  // whether the box generated can be broken
  bool _breakable;
  // atoms to be displayed horizontally next to each-other
  std::vector<sptr<Atom>> _elements;
  // previous atom (for nested Row atoms)
  sptr<AtomDecor> _previousAtom;

  /**
   * Change the atom-type to ORD if necessary
   * <p>
   * i.e. for formula: `$+ e - f$`, the plus sign should be treat as
   * an ordinary type
   */
  static void changeToOrd(AtomDecor* cur, AtomDecor* prev, Atom* next);

  sptr<CharSymbol> currentChar(int i);

  int processInvalid(
    const sptr<TextAtom>& txt,
    bool isMathMode,
    int i,
    Env& env
  );

  sptr<TextAtom> processContinues(int& i, bool isMathMode);

public:
  static bool _breakEveywhere;

  bool _lookAtLastAtom;

  RowAtom() : _lookAtLastAtom(false), _breakable(true) {}

  explicit RowAtom(const sptr<Atom>& atom);

  /** Get the atom at the front in the elements */
  sptr<Atom> getFirstAtom();

  /** Get and remove the atom at the tail in the elements */
  sptr<Atom> popBack();

  /**
   * Get the atom at position
   *
   * @param pos the position of the atom to retrieve
   */
  sptr<Atom> get(size_t pos);

  /**
   * Indicate the box generated by this atom can be broken or not
   *
   * @param breakable indicate whether the box can be broken
   */
  inline void setBreakable(bool breakable) {
    _breakable = breakable;
  }

  /** Get the size of the elements */
  inline size_t size() const {
    return _elements.size();
  }

  /** Push an atom to back */
  void add(const sptr<Atom>& atom);

  sptr<Box> createBox(Env& env) override;

  void setPreviousAtom(const sptr<AtomDecor>& prev) override;

  AtomType leftType() const override;

  AtomType rightType() const override;
};

}

#endif //LATEX_ATOM_ROW_H
