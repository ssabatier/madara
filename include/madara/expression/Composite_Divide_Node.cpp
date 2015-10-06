/* -*- C++ -*- */
#ifndef _DIVIDE_NODE_CPP_
#define _DIVIDE_NODE_CPP_

#ifndef _MADARA_NO_KARL_

#include <iostream>

#include "madara/expression/Composite_Binary_Node.h"
#include "madara/expression/Composite_Divide_Node.h"
#include "madara/expression/Visitor.h"
#include "madara/expression/Leaf_Node.h"


// Ctor
madara::expression::Composite_Divide_Node::Composite_Divide_Node (
  logger::Logger & logger,
  Component_Node *left,
  Component_Node *right)
: Composite_Binary_Node (logger, left, right)
{    
}

// Dtor
madara::expression::Composite_Divide_Node::~Composite_Divide_Node (void)
{
}

madara::Knowledge_Record
madara::expression::Composite_Divide_Node::item (void) const
{
  madara::Knowledge_Record record;
  record.set_value ("/");
  return record;
}

/// Prune the tree of unnecessary nodes. 
/// Returns evaluation of the node and sets can_change appropriately.
/// if this node can be changed, that means it shouldn't be pruned.
madara::Knowledge_Record
madara::expression::Composite_Divide_Node::prune (bool & can_change)
{
  bool left_child_can_change = false;
  bool right_child_can_change = false;
  madara::Knowledge_Record left_value;
  madara::Knowledge_Record right_value;
  madara::Knowledge_Record zero;

  if (this->left_)
  {
    left_value = this->left_->prune (left_child_can_change);
    if (!left_child_can_change && dynamic_cast <Leaf_Node *> (left_) == 0)
    {
      delete this->left_;
      this->left_ = new Leaf_Node (*(this->logger_), left_value);
    }
  }
  else
  {
    madara_logger_ptr_log (logger_, logger::LOG_EMERGENCY,
      "KARL COMPILE ERROR: Division has no left expression\n");
    exit (-1);  
  }

  if (this->right_)
  {
    right_value = this->right_->prune (right_child_can_change);

    if (!right_child_can_change && dynamic_cast <Leaf_Node *> (right_) == 0)
    {
      // leave this check which is important
      if (right_value.is_false ())
      {
        madara_logger_ptr_log (logger_, logger::LOG_EMERGENCY,
          "KARL COMPILE ERROR: Division results in permanent divide by zero\n");

        exit (-1);
      }
      // the only time we should delete right is if we have a clean division
      if (!left_child_can_change && 
          left_value % right_value == zero)
      {
        // don't worry about allocating anything. This is about to be
        // reclaimed anyway since !right_can_change and !left_can_change
        delete this->right_;
        this->right_ = 0;
      }
      else
        right_child_can_change = true;
    }

    //if (!right_child_can_change)
    //  right_child_can_change = true;
  }
  else
  {
    madara_logger_ptr_log (logger_, logger::LOG_EMERGENCY,
      "KARL COMPILE ERROR: Division has no right expression (divide by zero)\n");
    exit (-1);
  }

  can_change = left_child_can_change || right_child_can_change;

  return left_value / right_value;
}

/// Evaluates the node and its children. This does not prune any of
/// the expression tree, and is much faster than the prune function
madara::Knowledge_Record 
madara::expression::Composite_Divide_Node::evaluate (
const madara::knowledge::Knowledge_Update_Settings & settings)
{
  // only evaluate right if left evaluates to non-zero (0/{any_number} = 0)
  madara::Knowledge_Record lvalue (left_->evaluate (settings));
  madara::Knowledge_Record zero;
  if (lvalue.is_true ())
    return lvalue / right_->evaluate (settings);

  // note that we are not handling divide by zero. Still unsure whether I
  // want to use exceptions throughout evaluation or not. This should be as
  // quick as possible. We should probably only try to catch exception like
  // things in prune

  return zero;
}

// accept a visitor
void 
madara::expression::Composite_Divide_Node::accept (Visitor &visitor) const
{
  visitor.visit (*this);
}

#endif // _MADARA_NO_KARL_

#endif /* _DIVIDE_NODE_CPP_ */