/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_PRINTER_HPP_
#define MFG_PRINTER_HPP_

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include <iomanip>

namespace mfg_internal
{
using namespace mfg_pal;

inline void Indent( std::ostream &out, int level )
{
  for (auto i: NRange( level ))
  {
    N_UNUSED( i );
    out << " ";
  }
}

//////////////////////////
// PrintIRTree関連
//////////////////////////
inline void PrintVExpr( std::ostream &out, const IRElem* root );
inline void PrintVExprList( std::ostream& out, ExprVector* exprs );
inline void PrintExpr( std::ostream& out, const IRElem* stmtElem );

struct Separator {
  std::ostream& _out;
  std::string _sep;
  bool _isFirst = true;
  Separator( std::ostream& out ) : Separator( out, ", ")  {}
  Separator( std::ostream& out, const std::string& sep ) : _out( out ), _sep( sep ) {}

  void MaySep()
  {
    if (_isFirst)
    {
      _isFirst = false;
      return;      
    }
    _out << _sep;
  }
};

inline void PrintIRElem( std::ostream &out, const IRElem* one )
{
  switch( one->_elemType )
  {
    case IRElemType::ImmElem:
    {
      auto op = one->As<ImmElem>();
      if (op->_type.IsInt())
      {
        out << op->Value<int32_t>();
      }
      else if (op->_type.IsUInt())
      {
        out << "u" << op->Value<uint32_t>();
      }
      else
      {
        assert( op->_type.IsFloat());
        out << std::fixed << std::setprecision(2) << op->Value<float>();
      }
      return;
    }
    case IRElemType::Variable:
    {
      auto v = one->As<Variable>();      
      out << v->_name;
      return;
    }
    case IRElemType::VectorElem:
      out << "VectorElem" << std::endl;
    break;
    default:
      out << (int)one->_elemType << std::endl;
    break;
  }
}
inline void PrintTerm( std::ostream &out, const IRElem* expr );

inline void PrintInfixBinOp( std::ostream &out, BinOp* elem )
{
  auto left = elem->GetLeft();
  auto right = elem->GetRight();
  auto opstr = elem->DisplayName();

  PrintTerm( out, left );
  out << " " << opstr << " ";
  PrintTerm( out, right );  
}

/*
  stmtっぽくない、インデントとかを考えなくて良いようなExprのPrint。
  ただし、BlockElemなど曖昧なものも含むので名前がVExpr（Valued Exprの略）というようなものになっている。
*/
inline void PrintVExpr( std::ostream &out, const IRElem* root )
{
  if( root->_node == nullptr)
  {
    PrintIRElem( out, root );
    return;
  }

  for( auto iter = root->_node->begin(); iter != root->_node->end(); iter++ )
  {
    if( iter.IsLeading() )
    {
      auto& elem = iter.GetContent();

      auto elemType = elem->_elemType;
      switch( elemType )
      {
        case IRElemType::BinOp:
        {
          auto binOp = elem->As<BinOp>();
          PrintInfixBinOp( out, binOp );
          iter.ToTrailing();
          continue;
        }
        case IRElemType::Call:
        {
          auto op = elem->As<Call>();
          out << op->DisplayName() << "(";
          PrintVExprList( out, op->GetArgs() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::TensorCall:
        {
          auto op = elem->As<TensorCall>();
          switch ( op->_mtype )
          {
          case TensorCall::TO_NCOORD:
            out << op->GetTensorInfo().Name() << ".to_ncoord(";
            PrintVExprList( out, op->GetArgs() );
            out << ")";
            break;
          case TensorCall::IS_INSIDE:
            out << op->GetTensorInfo().Name() << ".is_inside(";
            PrintVExprList( out, op->GetArgs() );
            out << ")";
            break;
          case TensorCall::TUPLE_LOAD:
            out << op->GetTensorInfo().Name() << "[" << op->_tupleIdx << "]";
            out << "(";
            PrintVExprList( out, op->GetArgs() );
            out << ")";
            break;
          case TensorCall::LOAD:
            out << op->GetTensorInfo().Name() << "(";
            PrintVExprList( out, op->GetArgs() );
            out << ")";
            break;
          }
          iter.ToTrailing();
          continue;
        }
        case IRElemType::SwizzleCall:
        {
          auto op = elem->As<SwizzleCall>();
          PrintVExpr( out, op->GetTargetTuple() );
          out << ".swizzle(";

          Separator sep( out );
          auto indices = op->_indices;
          for( auto i : indices )
          {
            sep.MaySep();
            out << i;
          }
          
          out << ")";

          iter.ToTrailing();
          continue;
        }
        case IRElemType::LoadExpr:
        {
          auto op = elem->As<LoadExpr>();

          out << (op->IsLocal() ? "load_loc:" : "load:");
          out << op->_bufName << "[";
          PrintTerm( out, op->GetIndexArg() );
          out << "]";

          iter.ToTrailing();
          continue;
        }
        case IRElemType::SamplerElem:
        {
          auto op = elem->As<SamplerElem>();
          out << "sampler<" << op->_target.Name() <<">(address=";
          switch( op->_addressType )
          {
            case SamplerElem::CLAMP_TO_EDGE:
            {
              out << ".ClampToEdge";
              break;
            }
            case SamplerElem::CLAMP_TO_BORDER_VALUE:
            {
              out << ".ClampToBorderValue";
              break;
            }
            case SamplerElem::NORMAL_EDGE:
            {
              out << ".None";
              break;
            }
          }
          out << ", coord=";
          switch( op->_coordType )
          {
            case SamplerElem::PIXEL_COORD:
            {
              out << ".Pixel";
              break;
            }
            case SamplerElem::NORMALIZED_COORD_NEARESTNEIGHBOR:
            {
              out << ".NormalizedNearest";
              break;
            }
            case SamplerElem::NORMALIZED_COORD_BILINEAR:
            {
              out << ".NormalizedLinear";
              break;
            }
          }
          out << ", borer_value=";
          PrintVExprList( out, op->GetEdgeArgs() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::SamplerCall:
        {
          auto op = elem->As<SamplerCall>();
          out << "dec_call(" ;
          PrintVExprList( out, op->GetArgs() );
          out << ") : with_def {";
          PrintVExpr( out, op->GetDef() );
          out << "}";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::SpreadElem:
        {
          auto op = elem->As<SpreadElem>();
          out << "spread(";
          PrintVExpr( out, op->GetTarget() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::EnumElem:
        {
          auto op = elem->As<EnumElem>();
          out << "enum(" << op->_symId << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::IfEl:
        {
          auto op = elem->As<IfEl>();
          out << "ifel(";
          PrintVExpr( out, op->GetCondition() );
          out << ", ";
          PrintVExpr( out, op->GetTrueValue() );
          out << ", ";
          PrintVExpr( out, op->GetFalseValue() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::ReduceSum:
        {

          auto op = elem->As<ReduceSum>();
          out << "reduce_sum " << op->_name << " ";

          auto ranges = op->GetRanges()->ShallowCopy<RangeElem>();

          Separator sep( out );
          for (auto prange: ranges)
          {
            sep.MaySep();
            PrintVExpr( out, prange->GetBegin() );
            out << "..<";
            PrintVExpr( out, prange->GetEnd() );
          }
          out << " by (";

          PrintExpr( out, op->GetBlock() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::Reduce:
        {

          auto op = elem->As<Reduce>();
          out << "reduce " << op->_name << " init=";
          PrintVExpr( out, op->GetInitValue() );
          out << ", ";

          auto ranges = op->GetRanges()->ShallowCopy<RangeElem>();

          Separator sep( out );
          for (auto prange : ranges )
          {
            sep.MaySep();
            PrintVExpr( out, prange->GetBegin() );
            out << "..<";
            PrintVExpr( out, prange->GetEnd() );
          }
          out <<" by (";

          PrintExpr( out, op->GetBlock() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::ReduceToScalar:
        {
          auto op = elem->As<ReduceToScalar>();
          
          out <<"reduce_to_scalar<" << op->GetSrcTensorName() << ">.accumulate(dim=";
          PrintVExpr( out, op->GetTargetDim() );
          out << ", init=";
          PrintVExpr( out, op->GetInitValue() );
          out << ", block=";
          PrintExpr( out, op->GetBlock() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::TensorIterator:
        {
          auto op = elem->As<TensorIterator>();
          out << op->TensorName() << "." << op->DisplayName() << " " << op->_name << "(";
          PrintExpr( out, op->GetBlock() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        case IRElemType::TupleElem:
        {
          auto op = elem->As<TupleElem>();
          out << "tuple(";
          PrintVExprList( out, op->GetItems() );
          out << ")";
          iter.ToTrailing();
          continue;
        }
        default:
        {
          PrintIRElem( out, elem.get() );
          continue;
        }
      }
    }
  }
}

/*
  PrintExprに、必要ならカッコでくくる。
*/
inline void PrintTerm( std::ostream &out, const IRElem* term )
{
  if (term->HasChildren() && term->_elemType != IRElemType::Call)
  {
    out << "(";
    PrintVExpr( out, term );
    out << ")";
  }
  else
  {
    PrintVExpr( out, term );
  }
}

inline void PrintVExprList( std::ostream& out, ExprVector* exprs )
{
  Separator sep( out );
  for( auto& expr : *exprs )
  {
    sep.MaySep();
    PrintVExpr( out, expr.get() );
  }
}

struct EnclosedWith
{
  std::ostream& _out;
  const char* _open;
  const char* _close;
  bool _enabled;

  EnclosedWith( std::ostream& out, const char* open, const char* close, bool enabled = true ) : _out( out ), _open( open ), _close( close ), _enabled( enabled )
  {
    if (_enabled)
    {
      _out << _open;
    }
  }

  ~EnclosedWith()
  {
    if (_enabled)
    {
      _out << _close;
    }
  }

};

inline void PrintExpr( std::ostream& out, const IRElem* stmtElem )
{
  int indent = 0;
  auto root = stmtElem->_node;
  assert( root != nullptr );
  for (auto iter = root->begin(); iter != root->end(); iter++)
  {
    auto& content = iter.GetContent();
    if (content->_elemType == IRElemType::Let)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<Let>();
        Indent( out, indent );
        out << "let " << op->_name << " = ";
        PrintVExpr( out, op->GetValue() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::VectorLet)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<VectorLet>();
        Indent( out, indent );
        out << "letv " << op->_name << " = ";
        PrintVExpr( out, op->GetValue() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::TupleLet)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<TupleLet>();
        Indent( out, indent );
        out << "let [";

        Separator sep( out );
        for( auto& name: op->_names )
        {
          sep.MaySep();
          out << name;
        }
        out << "] = ";
        PrintVExpr( out, op->GetValue() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::VarLet)
    {
      if (iter.IsLeading())
      {
        // Letとほぼ同じ。
        auto op = content->As<VarLet>();
        Indent( out, indent );
        out << "var " << op->_name << " = ";
        PrintVExpr( out, op->GetValue() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::VectorElem)
    {
       // ExprVector
      continue; // 単に子供に進んでいくだけでなにも出力しない。
    }
    else if (content->_elemType == IRElemType::BodyElem)
    {
      if (iter.IsLeading())
      {
        Indent( out, indent );
        out << "vblock {" << std::endl;
        indent++;
      }
      else
      {
        indent--;
        Indent( out, indent );
        out << "}" << std::endl;
      }
    }
    else if (content->_elemType == IRElemType::ForStmt)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<ForStmt>();
        Indent( out, indent );
        out << "for(" << op->_varName << ": begin=";
        PrintVExpr( out, op->GetBeginExpr() );
        out << ", end=";
        PrintVExpr( out, op->GetEndExpr() );
        out << ") {" << std::endl;
        indent++;
        Indent( out, indent );

        ForStmt::SkipToBody( iter );        
      }
      else
      {
        indent--;
        Indent( out, indent );
        out << "}" << std::endl;
      }
    }
    else if (content->_elemType == IRElemType::ExecTensorInitialize)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<ExecTensorInitialize>();
        Indent( out, indent );

        out << "realize_tensor " << op->_rname << ": (";

        Separator sep(out);
        for (size_t dim : op->_bounds)
        {
          sep.MaySep();
          out << dim;
        }
        out << ")" << std::endl;

        indent++;
        Indent( out, indent );
      }
      else
      {
        indent--;
      }
    }
    else if (content->_elemType == IRElemType::TensorIterator && content->_type.IsVoid() )
    {
      if (iter.IsLeading())
      {
        auto op = content->As<TensorIterator>();
        assert( op->_itype == TensorIterator::EXEC_FOREACH );

        Indent( out, indent );

        out << op->TensorName() << "." << op->DisplayName() << std::endl;

        // 次のBlockElemのためにインデントする
        indent++;
        Indent( out, indent );
      }
      else
      {
        indent--;
      }
    }
    else if (content->_elemType == IRElemType::TransformTensor )
    {
      auto op = content->As<TransformTensor>();
      if (op->_method == TransformTensor::ACCM)
      {
        if (iter.IsLeading())
        {
          TransAccmAccessor acc( *op );

          Indent( out, indent );

          out << op->TensorName() << ".trans_accm!(";
          PrintVExpr( out, acc.GetTargetDim() );
          out << ")" << std::endl;

          // 次のBlockElemのためにインデントする
          indent++;
          Indent( out, indent );
          TransAccmAccessor::SkipToBlock( iter );

        }
        else
        {
          indent--;
        }
      }
      else
      {
        assert( op->_method == TransformTensor::SORT );
        if (iter.IsLeading())
        {
          Indent( out, indent );
          out << op->TensorName() << ".trans_sort!(";
          PrintVExpr( out, op->GetArg( 0 ) );
          out << ")" << std::endl;
          iter.ToTrailing();
          continue;
        }
      }
    }
    else if (content->_elemType == IRElemType::CallStmt)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<CallStmt>();

        Indent( out, indent );
        out << "callstmt: " << op->Name() << "(";
        PrintVExprList( out, op->GetArgs() );
        out << ")" << std::endl;
        iter.ToTrailing();
        continue;
      }
    }
    else if (content->_elemType == IRElemType::DefByReduce )
    {
      if (iter.IsLeading())
      {
        auto op = content->As<DefByReduce>();

        Indent( out, indent );

        
        out << "def " << op->GetTensorInfo().Name() << " by reduce<" << op->GetSrcTensorName() << ">.accumulate(dim=";
        PrintVExpr( out, op->GetTargetDim() );
        out << ", init=";
        PrintVExpr( out, op->GetInitValue() );
        out << ")" << std::endl;

        // 次のBlockElemのためにインデントする
        indent++;
        Indent( out, indent );
        DefByReduce::SkipToBlock( iter );
      }
      else
      {
        indent--;
      }
    }
    else if (content->_elemType == IRElemType::CompoundAssignment)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<CompoundAssignment>();
        Indent( out, indent );

        if (op->IsAfterLower())
        {
          // LoadExprは便宜上使っているだけなので、出力は手動でやる。
          auto lload = op->GetLeftAfterLower();
          out << "mut " << lload->_bufName << "[";
          PrintVExpr( out, lload->GetIndexArg() );
          out << "]";
        }
        else
        {
          out << "mut ";
          PrintVExpr( out, op->GetLeft() );
        }

        
        out << " += ";
        PrintVExpr( out, op->GetRight() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::StoreStmt)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<StoreStmt>();
        Indent( out, indent );

        out << "store " << op->_bufName << "[";
        PrintVExpr( out, op->GetIndexArg() );
        out << "] = ";
        PrintVExpr( out, op->GetRight() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::VarStore)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<VarStore>();
        Indent( out, indent );

        out << "store " << op->_varName << " = ";
        PrintVExpr( out, op->GetRight() );
        out << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::AllocateStmt)
    {
      if (iter.IsLeading())
      {
        auto op = content->As<AllocateStmt>();
        Indent( out, indent );

        out << "allocate " << op->_bufName << ":" << op->_bufType << "[";
        PrintVExpr( out, op->GetArg() );
        out << "]" << std::endl;
        iter.ToTrailing();
      }
    }
    else if (content->_elemType == IRElemType::BlockElem)
    {
      // BlockElemは少し特殊で、誰かの子供としてprintされるので、最初にindentなどはされている。
      // 一方でその中はブロック的なものなのでStmt的な要素もあるし、
      // ts.for_eachなどはまさしくStmt的なものなのでPrintVExprでなくPrintExprで扱うべき。
      // その曖昧な立場によりどちらに置いても中途半端だが、こちらに置く事にする。
      if (iter.IsLeading())
      {
        // すぐ下がBodyElemなので、中括弧などはそちらに任せる。
        auto op = content->As<BlockElem>();
        out << "func(";

        Separator sep(out);
        for (auto& arg : op->_args)
        {
          sep.MaySep();
          out << arg.first << ":" << arg.second;          
        }
        out << ")" << std::endl;

        indent++;
      }
      else
      {
        indent--;
      }
    }
    else
    {
      Indent( out, indent );
      PrintVExpr( out, content.get() );
      out << std::endl;
      iter.ToTrailing();
    }
  }
}

inline void PrintExpr( std::ostream& out, const Expr& stmt )
{
  PrintExpr( out, stmt.GetBase() );
}

inline std::ostream &operator<<(std::ostream &stream, const Expr &ir)
{
  if (ir.IsDefined())
  {
    PrintExpr( stream, ir.GetBase() );
  }
  else
  {
    stream << "(undefined)" << std::endl;
  }
  return stream;
}

} ///< mfg_internal
#endif

