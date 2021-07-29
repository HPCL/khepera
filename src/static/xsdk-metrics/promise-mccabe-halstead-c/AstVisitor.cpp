/* A recursive visitor that visits all nodes.
 * Based on https://clang.llvm.org/docs/RAVFrontendAction.html
 */
#include <iostream>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h> 
#include <clang/AST/Type.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include "llvm/Support/raw_ostream.h"
#include <vector>


#include "utils.hpp"

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

class McCabeMetricsVisitor : public RecursiveASTVisitor<McCabeMetricsVisitor> 
{
// adopted directly from: promise.site.uottawa.ca/SERepository/datasets/pc1.arff
// -- Base Measures : 
// calculate the following metrics given a function declaration
// mu1         -- number of unique operators 
// mu2         -- number of unique operands 
// N1          -- total occurences of operators 
// N2          -- total occurences of operands 
// length      -- N = N1 + N2 
// vocabulary  -- mu = mu1 + mu2 
// mu1'        -- 2 = potential operator count (function name and "return" operator) 
// mu2'        -- potential operand count. (number of arguments to the function) 
// For example, the expression "return max(w+x, x+y)" has "N1=4" 
// (i.e operators "return, max, +, +"), "N2 = 4" (i.e operands "w, x, x, y") 
// "mu1 = 3" (i.e unique operators "return, max, +") and "mu2=3" (i.e 
// unique operands "w, x, y") 
//
//
// ------------------------------------ 
// Derived Measures: 
// - P = volume =  V = N * log2(mu) (the number of mental comparisons needed
//                                   to write a program of length N)
// - V* = volume on minimal implementation
//      = (2 + mu2') * log2(2 + mu2') 
// - L  = program length = V*/N 
// - D  = difficulty = 1/L 
// - I  = intelligence = L' * V* 
// - E  = effort to write program = V / L 
// - T  = time to write program = E / 18 seconds 

public:
    explicit McCabeMetricsVisitor(ASTContext *p_context)
      : context(p_context) {} 
    double mu1 = 0.0; 
    double mu2 = 0.0; 
    double N1  = 0.0; 
    double N2 = 0.0; 
    double N  = 0.0; 
    double mu = 0.0; 
    double mu1_p = 0.0; 
    double mu2_p = 0.0; 


    double V = 0.0; 
    double V_star = 0.0; 
    double L = 0.0; 
    double L_p = 0; 
    double D = 0.0; 
    double I = 0.0; 
    double E = 0.0; 
    double T = 0.0; 
    double binops = 0.0; 

    bool VisitBinaryOperator(BinaryOperator* binop){
      N2                += 2; 

      mu1++; 
      N1++;
      return true; 
    }

    bool VisitUnaryOperator(UnaryOperator* unop)
    { 
      N2                += 1; 

      mu1++; 
      N1++;
      return true; 
    }

    bool VisitConditionalOperator(ConditionalOperator* condop)
    { 
      N2 += 3; 

      mu1++; 
      N1++;
      return true; 
    }

    bool VisitCallExpr(CallExpr* callexpr)
    {
    unsigned num_args   = callexpr -> getNumArgs(); 
    N2                += num_args; 

    mu1++; 
    N1++; 
      return true; 
    }

    bool VisitBinaryConditionalOperator(BinaryConditionalOperator* bincondop)
    {
    N2 += 2; 

    mu1++; 
    N1++;
      return true; 
    }

    bool VisitVarDecl(VarDecl* var_D)
    {
      // if this vardecl is a functionDecl's child 
      // then the lhs is a new operand 
      // so increment number of unique operands 
      if (var_D -> isLocalVarDecl())
      {
        mu2++; 
      }

      return true; 
    }

    double log2(int n)
    {
      return log(n) / log(2); 
    }

    void resetMetrics()
    {
       mu1 = 0.0; 
       mu2 = 0.0; 
       N1  = 0.0; 
       N2 = 0.0; 
       N  = 0.0; 
       mu = 0.0; 
       mu1_p = 0.0; 
       mu2_p = 0.0; 


       V = 0.0; 
       V_star = 0.0; 
       L = 0.0; 
       L_p = 0; 
       D = 0.0; 
       I = 0.0; 
       E = 0.0; 
       T = 0.0; 
    }

    bool VisitFunctionDecl(FunctionDecl* func_D)
    { 
      resetMetrics();
      cout << "------------------\n";
      cout << "Function name: " << func_D -> getNameInfo().getName()
                                                          .getAsString() 
                                                          << "\n";
      cout << "------------------\n"; 
      if(func_D -> hasPrototype()){
        mu1_p++; 
        QualType func_D_decl_retT = func_D -> getDeclaredReturnType(); 
        // const IdentifierInfo* t_str = func_D_decl_retT  
        //                                       ->  getBaseTypeIdentifier(); 
        // if (t_str -> getName().str().find("void") == std::string::npos){ // return type is not 
        //                                               // void so if we trust 
        //                                               // the type checker
        //                                               // return keyword must 
        //                                               // exist thus increment 
        //                                               // all relevant metrics
        //   mu1++; 
        //   N1++; 
        //   mu1_p++;  
        // }
        // find number of arguments 
        unsigned func_D_param_n = func_D -> getNumParams(); 
        mu2_p                  += func_D_param_n; 
        N2                     += func_D_param_n; 
        mu2                    += func_D_param_n; 
        if (func_D -> hasBody())
        {
          cout << "function has body\n"; 
          Stmt* func_D_body       = func_D -> getBody();
          this -> TraverseStmt(func_D_body); 
        } 
      } 
      N  = N1 + N2; 
      mu = mu1 + mu2; 

      // -- 
      V = N * log2(mu); 
      V_star = (2 + mu2_p) * log2(2 + mu2_p); 
      L      = V_star / N; 
      D      = 1 / L; 
      L_p    = 1 / D; 
      I      = L_p * V_star; 
      E      = V / L; 
      T      = E / 18; // 18 seems magical. time to make a mental comparison 
      //                 // for a program of length N? 
      cout << "number of unique operators mu1 : "   << mu1 << "\n"; 
      cout << "number of unique operands mu2 : "   << mu2 << "\n";
      cout << "Total operators N1 : "    << N1 << "\n";
      cout << "Total operands N2 : "    << N2 << "\n";
      cout << "N = N1 + N2 : "     << N << "\n";
      cout << "mu = mu1 + mu2 : "    << mu << "\n";
      cout << "potential operator count mu1_p : " << mu1_p << "\n";
      cout << "potential operand count mu2_p : " << mu2_p << "\n";
      return true; 
    }

private:
    ASTContext *context;
};


class McCabeMetricsConsumer : public clang::ASTConsumer {
public:
  explicit McCabeMetricsConsumer(ASTContext *context)
    : visitor(context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &context) {
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    // print structure with metrics per function
  }
private:
  McCabeMetricsVisitor visitor;
};

class McCabeMetricsAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &compiler, llvm::StringRef inFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new McCabeMetricsConsumer(&compiler.getASTContext()));
  }
};


static llvm::cl::OptionCategory ctCategory("ast-traverse options");

int main(int argc, const char **argv)
{

    /* From week2/clang-babysteps, this processes the source code as a string argument (not file)
    if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<McCabeMetricsAction>(), argv[1]);
    }
    */

    auto expectedParser = CommonOptionsParser::create(argc, argv, ctCategory);
    if (!expectedParser) {
       // Fail gracefully for unsupported options.
       llvm::errs() << expectedParser.takeError();
       return 1;
    }
    CommonOptionsParser& optionsParser = expectedParser.get();

    for(auto &sourceFile : optionsParser.getSourcePathList())
    {
       if(utils::fileExists(sourceFile) == false)
       {
          llvm::errs() << "File: " << sourceFile << " does not exist!\n";
          return -1;
       }

       auto sourcetxt = utils::getSourceCode(sourceFile);
       auto compileCommands = optionsParser.getCompilations().getCompileCommands(getAbsolutePath(sourceFile));

       
       std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
       //compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv[0]));
       for(auto &s : compileArgs)
          llvm::outs() << s << "\n";

       auto xfrontendAction = std::make_unique<McCabeMetricsAction>();
       utils::customRunToolOnCodeWithArgs(move(xfrontendAction), compileArgs, sourceFile);
   }
   return 0;

}
