#include "atom/atom_misc.h"
#include "atom/atom_delim.h"
#include "utils/utf.h"
#include "utils/string_utils.h"

using namespace std;
using namespace tex;

sptr<Box> BigDelimiterAtom::createBox(Env& env) {
  auto b = tex::createVDelim(_delim, env, _size);
  const auto axis = env.mathConsts().axisHeight() * env.scale();
  b->_shift = -(b->vlen() / 2 - b->_height) - axis;
  return b;
}

float OvalAtom::_multiplier = 0.5f;
float OvalAtom::_diameter = 0.f;

RotateAtom::RotateAtom(const sptr<Atom>& base, float angle, const wstring& option)
  : _angle(0), _option(Rotation::bl), _xunit(UnitType::em), _yunit(UnitType::em), _x(0), _y(0) {
  _type = base->_type;
  _base = base;
  _angle = angle;
  const string x = wide2utf8(option);
  const auto& opt = parseOption(x);
  auto it = opt.find("origin");
  if (it != opt.end()) {
    _option = RotateBox::getOrigin(it->second);
    return;
  }
  it = opt.find("x");
  if (it != opt.end()) {
    auto[u, x] = Units::getLength(it->second);
    _xunit = u, _x = x;
  } else {
    _xunit = UnitType::point, _x = 0;
  }
  it = opt.find("y");
  if (it != opt.end()) {
    auto[u, y] = Units::getLength(it->second);
    _yunit = u, _y = y;
  } else {
    _yunit = UnitType::point, _y = 0;
  }
}

RotateAtom::RotateAtom(const sptr<Atom>& base, const wstring& angle, const wstring& option)
  : _angle(0), _option(Rotation::none), _xunit(UnitType::em), _yunit(UnitType::em), _x(0), _y(0) {
  _type = base->_type;
  _base = base;
  valueof(angle, _angle);
  const string x = wide2utf8(option);
  _option = RotateBox::getOrigin(x);
}

sptr<Box> RotateAtom::createBox(Env& env) {
  if (_option != Rotation::none) {
    return sptrOf<RotateBox>(_base->createBox(env), _angle, _option);
  }

  const auto x = Units::fsize(_xunit, _x, env);
  const auto y = Units::fsize(_yunit, _y, env);
  return sptrOf<RotateBox>(_base->createBox(env), _angle, x, y);
}

void LongDivAtom::calculate(vector<wstring>& results) const {
  long quotient = _dividend / _divisor;
  results.push_back(towstring(quotient));
  string x = tostring(quotient);
  size_t len = x.length();
  long remaining = _dividend;
  results.push_back(towstring(remaining));
  for (size_t i = 0; i < len; i++) {
    long b = (x[i] - '0') * pow(10, len - i - 1);
    long product = b * _divisor;
    remaining = remaining - product;
    results.push_back(towstring(product));
    results.push_back(towstring(remaining));
  }
}

LongDivAtom::LongDivAtom(long divisor, long dividend)
  : _divisor(divisor), _dividend(dividend) {
  _halign = Alignment::right;
  setAlignTop(true);
  vector<wstring> results;
  calculate(results);

  auto rule = sptrOf<RuleAtom>(UnitType::ex, 0.f, UnitType::ex, 2.6f, UnitType::ex, 0.5f);

  const int s = results.size();
  for (int i = 0; i < s; i++) {
    auto num = Formula(results[i])._root;
    if (i == 1) {
      wstring divisor = towstring(_divisor);
      auto rparen = SymbolAtom::get("rparen");
      auto big = sptrOf<BigDelimiterAtom>(rparen, 1);
      auto ph = sptrOf<PhantomAtom>(big, false, true, true);
      auto ra = sptrOf<RowAtom>(ph);
      auto raised = sptrOf<RaiseAtom>(
        big,
        UnitType::x8,
        3.5f,
        UnitType::x8,
        0.f,
        UnitType::x8,
        0.f
      );
      ra->add(sptrOf<SmashedAtom>(raised));
      ra->add(num);
      auto oa = sptrOf<OverUnderBar>(ra, true);
      auto row = sptrOf<RowAtom>(Formula(divisor)._root);
      row->add(sptrOf<SpaceAtom>(SpaceType::thinMuSkip));
      row->add(oa);
      append(row);
      continue;
    }
    if (i % 2 == 0) {
      auto row = sptrOf<RowAtom>(num);
      row->add(rule);
      if (i == 0) append(row);
      else append(sptrOf<OverUnderBar>(row, true));
    } else {
      auto row = sptrOf<RowAtom>(num);
      row->add(rule);
      append(row);
    }
  }
}

sptr<Box> CancelAtom::createBox(Env& env) {
  return StrutBox::empty();
}