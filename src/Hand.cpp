#include "Hand.hpp"

#include <stdexcept>

void Hand::addCard(const Card &c) { cards_.push_back(c); }

void Hand::clear() { cards_.clear(); }

int Hand::size() const { return (int)cards_.size(); }




const Card &Hand::at(int i) const {
  if (i < 0 || i >= size())
    throw std::out_of_range("Hand::at: bad index");
  return cards_[i];
}




void Hand::replace(int i, const Card &c) {
  if (i < 0 || i >= size())
    throw std::out_of_range("Hand::replace: bad index");
  cards_[i] = c;
}
