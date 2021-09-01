# include <iostream>
# include <stdio.h>
# include <string>
# include <vector>
# include <sstream>
# include <string.h>
# include <stdlib.h>
# include <stack>
using namespace std ;

enum TerminalToken { 
  LEFT_PAREN, RIGHT_PAREN, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, ENTER, COMMENT, EOFILE
};
enum FunctionType { 
  NONE, CONS, QUOTE_FN, DEFINE, PART_ACCESSOR, PRIMITIVE_PREDICATE, OPERATOR_FN, EQU_TEST, BEGIN_FN, 
  COND_FN, CLEAN_ENVIRONMENT, EXIT, CUSTOMIZE, BEEN_QUOTE, LET, LAMBDA, VERBOSE
};

struct Token{
  string str ; // セ
  bool error ; // 琌岿粇 
  string errormsg ; // 岿粇癟 
  TerminalToken type ; // だ摸
  FunctionType funcType ; // ㄧΑだ摸 
  int lastIndexOfLine ; // (vector)gLineいindex(程char┮竚) 
}; 

struct Node{
  Token token ;
  string str ; // セ
  Node * left ; // オpointer 
  Node * right ; // pointer
  int ans_int ;
  float ans_float ; // 琌float碞钡float笲衡 
};

typedef Node * NodePtr ;

struct Symbol{
  string str ; // symbol嘿
  Node * value ; // Symbol璶砆﹚竡 
};
struct Fn{
  string str ; // Function嘿
  int numOfArgs ; // Τぶ把计
  string originFn ; // 硂琌砆functionn┮﹚竡 
  vector<string> args ; // 把计(嘿)硂柑 
  Node * value ; // 赣fn﹚竡(璶暗ㄆ)
};



int gTestNum = 0 ;
bool gVerbose = true ; // 疭璶龟...蛤define clean evn.璶ぃ璶Τ闽 
vector<string> gLine ; // 弄狥﹁(膀セ琌︽) 
vector<Token> gToken ; // 璶ちTokenToken﹃
vector<Symbol> gSymbolTAB ; // symbol Table ﹚竡筁symbol常穦硂柑 
vector<Fn> gFunctionTAB ; // ﹚竡筁function常硂柑 矗ㄑ琩高ノ 
vector<Symbol> glocal ; // let穦ノlocal variable常蹲硂柑 


NodePtr gTree ; // 攫攫セ攫 

// ------------------------------------------------------------------------------------------------
class OurScheme{
  public:  

  bool IsSeparator( string str ) {
    if ( str == " " || str == "\t" || str == "\n" || str == "(" || str == ")" || str == "'" 
         || str == "\"" || str == ";" ) {
      return true ;
    } // if 

    return false ;
  } // IsSeparator() 

  bool IsEscape( int index ) { // ' " \ 硂case惠璶escape (\n, \t痙帝)   
    if ( gLine.at( index ) == "'"  || gLine.at( index ) == "\"" || gLine.at( index ) == "\\"
         || gLine.at( index ) == "n"  || gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscape()

  bool IsEscapeN( int index ) { // ' " \ 硂case惠璶escape (\n, \t痙帝)   
    if ( gLine.at( index ) == "n" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeN()

  bool IsEscapeT( int index ) { // ' " \ 硂case惠璶escape (\n, \t痙帝)   
    if ( gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeT()

  bool IsNumber( string temp ) { // 把σhttps://www.itread01.com/content/1546245258.html 
    char ch ;
    stringstream ss( temp ) ;
    double isDouble ;
    
    if ( ! ( ss >> isDouble ) ) { // ss >> isDoubleр ss锣传Θdouble跑计
      return false ; // (int/ float常钡)肚传ア毖玥0 
    } // if 
    
    if ( ss >> ch ) { // 浪代岿粇块(计﹃)(ㄒ12.a
      return false ; // 钡Μ.a场だ (兵ンΘミ┮return false) 
    } // if 

    return true ;
  } // IsNumber()
  
  int CountEnter( int index ) { // 衡ヘ玡﹟ゼ矪瞶狥﹁いΤ碭传︽ 
    int counter = 0 ;
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gToken.size() && toBreak == false ) {
      if ( gToken.at( i ).str == "\n" ) {
        counter++ ;
      } // if

      if ( i == index ) { // 笿セ  1(︽)挡 
        counter++ ;
        toBreak = true ;
      } // if

      i++ ; 
    } // while 

    return counter ;
  } // CountEnter()


  void Classify( string temp, int &indexOfLine, int &indexOfToken, bool &error  ) {

    if ( temp == " " || temp == "\t" ) {
      ;
    } // if
    else {
      Token gtemp ;
      gtemp.str = temp ;
      gtemp.error = false ;
      gtemp.errormsg = "" ;
      gtemp.lastIndexOfLine = indexOfLine ;
      gtemp.funcType = NONE ;
      if ( temp == "\n" ) { // 传︽ 
        gtemp.type = ENTER ;
      } // if 
      else if ( temp == "(" ) { // オ珹腹 
        gtemp.type = LEFT_PAREN ;
        int j = indexOfLine + 1 ;
        while ( j < gLine.size() && ( gLine.at( j ) == " " || gLine.at( j ) == "\t" ) ) j++ ; 
        if ( j < gLine.size() && gLine.at( j ) == ")" ) { // () nil猵 
          gtemp.type = NIL ;
          gtemp.str = "nil" ;
          indexOfLine = j ;
          gtemp.lastIndexOfLine = j ;
        } // if 
      } // else if (オ珹腹) 
      else if ( temp == ")" ) { // 珹腹 
        gtemp.type = RIGHT_PAREN ;
      } // else if (珹腹)
      else if ( temp == "'" ) { // (虫)ま腹 Quote 
        gtemp.type = QUOTE ;
        gtemp.funcType = QUOTE_FN ;
      } // else if (虫)ま腹 Quote
      else if ( temp == "nil" || temp == "#f" ) { // NIL
        gtemp.type = NIL ;
      } // else if (nil)
      else if ( temp == "t" || temp == "#t"  ) { // T
        gtemp.str = "#t" ;
        gtemp.type = T ;
      } // else if (T)
      else if ( temp == ";" ) { // 爹秆
        gtemp.type = COMMENT ;  
        while ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) != "\n" ) { // 弄传︽ 
          indexOfLine++ ;
        } // while

        if ( indexOfLine < gLine.size() ) 
          indexOfLine-- ; // 穦++, Ω传︽    
      } // else if (爹秆)
      else if ( temp == "\"" ) { // String 
        // string lastStr = "" ; // 笿TOKEN
        bool toBreak = false ; 
        gtemp.str = temp ; // "(蛮)ま腹            
        indexOfLine++ ;
        while ( indexOfLine < gLine.size() && toBreak == false ) { // 弄材(蛮)ま腹 
          if ( gLine.at( indexOfLine ) == "\"" ) { // ﹃挡ま腹 
            toBreak = true ;
          } // if 
          else if ( gLine.at( indexOfLine ) == "\\" ) { // は弊絬 
            if ( indexOfLine + 1 >= gLine.size() ) toBreak = true ;
            else if ( IsEscape( indexOfLine + 1 ) ) { // 耞は弊絬琌escape種竡 
              indexOfLine++ ; // 铬筁Τescape﹚竡は弊絬  : ヘ玡 \"  "  
            } // else if 

            char enter = '\n' ; // 传︽
            char tab = '\t' ;
            if ( toBreak == false ) {
              if ( IsEscapeN( indexOfLine ) ) {
                gtemp.str = gtemp.str + enter ;
                indexOfLine++ ;
              } // if 
              else if ( IsEscapeT( indexOfLine ) ) {
                gtemp.str = gtemp.str + tab ;
                indexOfLine++ ;
              } // else if
              else {
                gtemp.str = gtemp.str + gLine.at( indexOfLine ) ;
                indexOfLine++ ; 
              } // else 
            } // if

              
          } // else if
          else if ( gLine.at( indexOfLine ) == "\n" ) {
            toBreak = true ;
          } // else if 
          else { // string场だ 
            gtemp.str = gtemp.str + gLine.at( indexOfLine ) ;
            indexOfLine++ ;
          } // else            
        } // while 
      
        if ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) == "\"" ) { // STRINGΑタ絋       
          gtemp.str = gtemp.str + gLine.at( indexOfLine ) ;
          gtemp.type = STRING ; 
        }  // if 
        else { // STRING ERROR
          error = true ;
          gtemp.error = true ; 
          string enterCounter = "" ;
          IToA( CountEnter( gToken.size() ) + 1, enterCounter ) ;
          string column = "" ;
          IToA( indexOfLine + 1, column ) ;
          gtemp.type = STRING ;
          gtemp.lastIndexOfLine = indexOfLine ;
          gtemp.errormsg = "ERROR (no closing quote) : END-OF-LINE encountered at Line " ;
          gtemp.type = STRING ; 
        } // else 
      } // else if (String)    
      else if ( IsNumber( temp ) ) { // 计(int/float) 

        if ( temp[0] == '+' ) {
          temp.erase( 0, 1 ) ; // 眖index = 0秨﹍埃1char
        } // if 


        gtemp.str = "" ;        
        bool isFloat = false ;
        int i = 0 ;
        int indexOfDot = 0 ;
        char lastCH = '\0' ;
        while ( i < temp.length() ) { // 耞琌int临琌float
 
          if ( temp[i] == '.' ) {
            indexOfDot = i ;
            isFloat = true ;
            if ( lastCH == '-' ) {
              gtemp.str = gtemp.str + "0" ;
            } // else if

            if ( i == 0 ) { // 璝琌.1 => 0.1 玡干箂 
              gtemp.str = "0" + gtemp.str ;
            } // if
 
          } // if   

          gtemp.str = gtemp.str + temp[i] ;
          lastCH = temp[i] ;
          i++ ;
        } // while 

        gtemp.type = INT ;
        if ( isFloat ) {       
          gtemp.type = FLOAT ;
          int num = temp.length() - indexOfDot - 1 ; // Τ碭计
          int n = 3 - num ; // 璶干倒0
          while ( n > 0 ) {
            gtemp.str = gtemp.str + "0" ;
            n-- ;
          } // while
    
        } // if  
      } // else if (计(int/float))
      else if ( temp == "." ) { // DOT
        gtemp.type = DOT ;
      } // else if 
      else { // SYMBOL
        gtemp.type = SYMBOL ;
        if ( gtemp.str == "cons" || gtemp.str == "list" ) gtemp.funcType = CONS ;
        else if ( gtemp.str == "quote" ) { // QUOTE
          gtemp.type = SYMBOL ;
          gtemp.funcType = QUOTE_FN ;
        } // else if QUOTE
        else if ( gtemp.str == "define" ) gtemp.funcType = DEFINE ;
        else if ( gtemp.str == "car" || gtemp.str == "cdr" ) gtemp.funcType = PART_ACCESSOR ;
        else if ( gtemp.str == "atom?" || gtemp.str == "pair?" || gtemp.str == "list?" || 
                  gtemp.str == "null?" || gtemp.str == "integer?" || gtemp.str == "real?" || 
                  gtemp.str == "number?" || gtemp.str == "string?" || gtemp.str == "boolean?" || 
                  gtemp.str == "symbol?" ) gtemp.funcType = PRIMITIVE_PREDICATE ;
        else if ( gtemp.str == "+" || gtemp.str == "-" || gtemp.str == "*" || gtemp.str == "/" ||
                  gtemp.str == "not" || gtemp.str == "and" || gtemp.str == "or" || gtemp.str == ">" ||
                  gtemp.str == ">=" || gtemp.str == "<" || gtemp.str == "<=" || gtemp.str == "=" || 
                  gtemp.str == "string-append" || gtemp.str == "string>?" || gtemp.str == "string<?" ||
                  gtemp.str == "string=?" ) gtemp.funcType = OPERATOR_FN ;
        else if ( gtemp.str == "eqv?" || gtemp.str == "equal?" ) gtemp.funcType = EQU_TEST ;
        else if ( gtemp.str == "begin" ) gtemp.funcType = BEGIN_FN ;
        else if ( gtemp.str == "if" || gtemp.str == "cond" ) gtemp.funcType = COND_FN ;
        else if ( gtemp.str == "clean-environment" ) gtemp.funcType = CLEAN_ENVIRONMENT ;
        else if ( gtemp.str == "exit" ) gtemp.funcType = EXIT ; 
        else if ( gtemp.str == "let" ) gtemp.funcType = LET ;
        else if ( gtemp.str == "lambda" ) gtemp.funcType = LAMBDA ;
        else if ( gtemp.str == "verbose" ) gtemp.funcType = VERBOSE ;
      } // else (SYMBOL)

      if ( gtemp.type != COMMENT ) 
        gToken.push_back( gtemp ) ;

      if ( error ) { // 璝string岿 玥程璶ENTER 
        gtemp.str = "\n" ; 
        gtemp.type = ENTER ;
        gToken.push_back( gtemp ) ;
      } // if 
    } // else 
  } // Classify()
    
  void GetToken( bool &error ) { // だ摸 => Token﹃  
    bool toBreak = false ;
    int indexOfLine = 0 ; // index of (vector) gLine(string)
    int indexOfToken = 0 ; // index of (vextor) gToken(Token)
    int countEnter = 0 ; // 衡Τ碭传︽ 
    string temp = "" ;
    while ( toBreak == false && indexOfLine < gLine.size() ) { // 临⊿だ摸Ч 

      if ( gLine.at( indexOfLine ) == "\n" ) { // 传︽
        if ( temp != "" ) { // 玡ΤToken璶矪瞶 
          indexOfToken++ ; // 传TOKEN 
          indexOfLine-- ; // 铬Tokenよ 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // 传︽硂柑 
          temp = "" ;
        } // if  


 
        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // 传TOKEN 
        Classify( temp, indexOfLine, indexOfToken, error ) ;
        temp = "" ; 
 
        toBreak = true ;
      } // if (传︽) 
      else if ( IsSeparator( gLine.at( indexOfLine ) ) ) { // Separator
        if ( temp != "" ) { // 玡ΤToken璶矪瞶 
          indexOfToken++ ; // 传TOKEN
          indexOfLine-- ; // 铬Tokenよ 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // 传︽硂柑
          temp = "" ;
        } // if 

        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // 传TOKEN 
        Classify( temp, indexOfLine, indexOfToken, error ) ;
        temp = "" ; 
      } // else if 
      else {
        temp = temp + gLine.at( indexOfLine ) ;
      } // else 
       
      if ( toBreak == false ) {
        indexOfLine++ ;
      } // if 
      
       
    } // while 

    if ( temp != "" ) {
      indexOfLine-- ;
      indexOfToken++ ; // 传TOKEN 
      Classify( temp, indexOfLine, indexOfToken, error ) ;
      temp = "" ;
    } // if  

    // Debug() ;    
  } // GetToken()

  bool IsATOM( TerminalToken type ) {
    if ( type == INT ) return true ;
    else if ( type == STRING ) return true ;
    else if ( type == FLOAT ) return true ;
    else if ( type == NIL ) return true ;
    else if ( type == T ) return true ;
    else if ( type == SYMBOL ) return true ;
    return false ;
  } // IsATOM() 

  void IToA( int num, string & strNum ) {
    int n = 0 ;
    while ( num > 0 ) {
      n = num % 10 ;
      num = num / 10 ;
      if ( n == 1 ) {
        strNum = "1" + strNum ;
      } // if
      else if ( n == 2 ) {
        strNum = "2" + strNum ;
      } // else if
      else if ( n == 3 ) {
        strNum = "3" + strNum ;
      } // else if 
      else if ( n == 4 ) {
        strNum = "4" + strNum ;
      } // else if 
      else if ( n == 5 ) {
        strNum = "5" + strNum ;
      } // else if 
      else if ( n == 6 ) {
        strNum = "6" + strNum ;
      } // else if 
      else if ( n == 7 ) {
        strNum = "7" + strNum ;
      } // else if 
      else if ( n == 8 ) {
        strNum = "8" + strNum ;
      } // else if 
      else if ( n == 9 ) {
        strNum = "9" + strNum ;
      } // else if 
      else if ( n == 0 ) {
        strNum = "0" + strNum ;
      } // else if  
    } // while 
  } // IToA()

  bool SyntaxAnalysis( int &index, bool & error, string & errormsg, bool & isComplete, 
                       bool & isFirstToken, bool & isEOF, bool & hasQuote ) { // ゅ猭だ猂 


    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // 传︽
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 


      index++ ;

    } // while 传︽

    if ( index < gToken.size() && gToken.at( index ).error == true ) { // 璝琌string岿粇 
      error = true ; 
      errormsg = gToken.at( index ).errormsg ;

      string enterCounter = "" ;
      IToA( CountEnter( index ), enterCounter ) ;
      string line = "" ;
      IToA( gToken.at( index ).lastIndexOfLine + 1, line ) ;
      string str = gToken.at( index ).str ;
      errormsg = errormsg + enterCounter + " Column " + line + "\n" ;

      return false ;
    } // if 

    if ( index < gToken.size() && gToken.at( index ).type == EOFILE ) {
      error = true ;
      isEOF = true ;
      return false ;
    } // if 

    if ( error == true || index >= gToken.size() ) { // 挡!┪琌弧ぃ暗ㄆ
      if ( index >= gToken.size() ) isComplete = false ;

      return true ; 
    } // if

    
    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // 传︽┪爹秆 
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 

      index++ ;

    } // while 传︽

    if ( index >= gToken.size() ) {
      isComplete = false ;
      return true ;
    } // if 
     
    if ( index < gToken.size() && IsATOM( gToken.at( index ).type ) ) { // Atom
      if ( isFirstToken ) {
        isComplete = true ;
        index++ ; 
        return true ;
      } // if

      isFirstToken = false ; 
      return true ; 
    } // if ATOM
    else if ( index < gToken.size() && gToken.at( index ).type == QUOTE ) { // QUOTE <S-exp>
      hasQuote = true ;
      index++ ;
      if ( ! SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote )  ) { 
        // ゅ猭岿钡挡
        error = true ;
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( index < gToken.size() && gToken.at( index ).type == LEFT_PAREN ) { 
      // オ珹腹 <S-exp>{<S-exp>}[DOT<S-exp>]珹腹
      isFirstToken = false ; 
      index++ ;
      int j = index ; 
      while ( j < gToken.size() && gToken.at( j ).type == ENTER ) { // peek
        j++ ;
      } // while

      if ( j >= gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 
 
      bool isNIL = false ;
      if ( j < gToken.size() && gToken.at( j ).type == RIGHT_PAREN ) { // ()猵 
        gToken.at( index - 1 ).str = "nil" ;
        gToken.at( index - 1 ).type = NIL ;
        gToken.at( index - 1 ).lastIndexOfLine = j ;
        int k = index ;
        while ( k <= j ) { // 防 
          gToken.erase( gToken.begin() + index ) ;
          k++ ;
        } // while 

        index = index - 1 ;
        isNIL = true ;
        return true ;
      } // if ()猵 
      else if ( ! SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ) 
        return false ;

      if ( isNIL == false )
        index++ ; // 耞 
      while ( index < gToken.size() && gToken.at( index ).type != DOT 
              && gToken.at( index ).type != RIGHT_PAREN ) { // <S-exp> 耞<S-exp>琌タ絋
        if ( index < gToken.size() && gToken.at( index ).type == ENTER ) index++ ; 
        else if ( ! SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ) 
          return false ;
        else index++ ;
      } // while 
      

      while ( index < gToken.size() && ( gToken.at( index ).type == ENTER ) ) { 
        index++ ;
      } // while 

  
      if ( index >= gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 
      else if ( index < gToken.size() && gToken.at( index ).type == DOT ) { // DOT钡 ATOM/ オ珹腹 / 虫ま腹 
        index++ ;
        if ( index >= gToken.size() ) {
          isComplete = false ;
          return true ;
        } // if 

        if ( index < gToken.size() && IsATOM( gToken.at( index ).type ) == false && 
             gToken.at( index ).type != LEFT_PAREN 
             &&  gToken.at( index ).type != QUOTE && gToken.at( index ).type != ENTER && 
             gToken.at( index ).type != COMMENT ) {
        // 璝DOTぃ琌ATOM / オ珹腹 / 虫ま腹 / 传︽ / 爹秆 
          error = true ;
          string enterCounter = "" ;
          IToA( CountEnter( index ), enterCounter ) ;
          string line = "" ;
          IToA( gToken.at( index ).lastIndexOfLine + 1, line ) ;
          string str = gToken.at( index ).str ;
          errormsg = "ERROR (unexpected token) : atom or '(' expected when token at Line "  + 
                      enterCounter + " Column " + line + " is >>" + str + "<<\n" ;
          return false ;
        } // if

        if (  index < gToken.size() && ( gToken.at( index ).type == ENTER || 
                                         gToken.at( index ).type == COMMENT ) ) {
          if ( index + 1 >= gToken.size() ) {
            isComplete = false ;
            return true ;
          } // if 
          else index++ ; // ┕耞

          if ( index < gToken.size() && gToken.at( index ).type == ENTER ) index++ ; 
        } // if 
        //  DOT钡 ATOM/ オ珹腹 / 虫ま腹 玥耞赣︽ゅ猭 
        if ( SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) == false ) 
          return false ;

        index++ ;
        
        while ( index < gToken.size() && ( gToken.at( index ).type == ENTER 
                                           || gToken.at( index ).type == COMMENT ) ) {
          index++ ;
        } // while

        if ( index >= gToken.size() ) {
          isComplete = false ;
          return true ;
        } // if 
        else if ( index < gToken.size() && gToken.at( index ).type == RIGHT_PAREN ) return true ;
        else if ( index < gToken.size() && gToken.at( index ).type != RIGHT_PAREN ) { // DOT<S-exp>⊿Τ珹腹
          while ( index < gToken.size() && ( gToken.at( index ).type == ENTER 
                                             || gToken.at( index ).type == COMMENT ) ) {
            index++ ; 
          } // while 

          if ( index < gToken.size() ) {
            error = true ;
            string enterCounter = "" ;
            IToA( CountEnter( index ), enterCounter ) ;
            string line = "" ;
            IToA( gToken.at( index ).lastIndexOfLine + 1, line ) ;
            string str = gToken.at( index ).str ;        
            errormsg = "ERROR (unexpected token) : ')' expected when token at Line " + enterCounter
                       + " Column " + line + " is >>" + str + "<<\n" ;
            return false ;
          } // if
          else return true ; 
        } // else if  
      } // else if // DOT钡 ATOM/ オ珹腹 / 虫ま腹
      else if ( index < gToken.size() && gToken.at( index ).type == RIGHT_PAREN ) { // 珹腹 ->挡
        isComplete = true ; 
        return true ;
      } // else if 珹腹 ->挡
      else if ( isNIL == false ) return false ;

    } // else if オ珹腹
    else if ( index < gToken.size() && gToken.at( index ).type == EOFILE ) {
      error = true ;
      isEOF = true ;
      return false ;
    } // if 
    else {
      error = true ; 
      string enterCounter = "" ;
      IToA( CountEnter( index ), enterCounter ) ;
      string line = "" ;
      IToA( gToken.at( index ).lastIndexOfLine + 1, line ) ;
      string str = gToken.at( index ).str ;
      errormsg = "ERROR (unexpected token) : atom or '(' expected when token at Line " + enterCounter 
               + " Column " + line + " is >>" + str + "<<\n" ;
      return false ; 
    } // else

    return true ;
  } // SyntaxAnalysis() 

  bool IsExitToken() {
    int i = 0 ;
    bool hasLP = false ;
    bool hasExit = false ;
    bool hasRP = false ;
    bool hasDOT = false ;
    bool hasNIL = false ;
    while ( i < gToken.size() ) {
      if ( gToken.at( i ).str == "\n" || gToken.at( i ).str == ";" ) ;
      else if ( gToken.at( i ).str == "(" ) {
        if ( hasLP == true ) return false ;
        hasLP = true ;
      } // else if
      else if ( gToken.at( i ).str == "exit" ) {
        if ( hasLP == true ) {
          hasExit = true ;
        } // if 
        else {
          return false ;
        } // else 
      } // else if
      else if ( gToken.at( i ).type == DOT ) {
        hasDOT = true ;
      } // else if 
      else if ( gToken.at( i ).type == NIL ) {
        if ( hasDOT ) hasNIL = true ;
        else return false ;
      } // else if 
      else if ( gToken.at( i ).str == ")" ) {
        if ( hasExit == true ) {
          if ( hasDOT && hasNIL ) return true ;
          else if ( hasDOT == false && hasNIL == false ) return true ;
          else return false ;
        } // if 
      
        return false ; 
      } // else if
      else {
        return false ;
      } // else 

      i++ ;
    } // while 

    return false ;
  } // IsExitToken()

  void BuildTree( NodePtr tree, int & i, bool isLeftChild, bool isRightChild, int & lastIndexOfToken, 
                  int & LP, int & RP, string parent, int limitIndex ) { // 攫 讽オ珹腹计癸嘿玥ボΘ攫 

    if ( i >= gToken.size() || i > limitIndex ) { // 挡
      ;
    } // if 挡
    else if ( i < gToken.size() && ! ( i > limitIndex ) &&  
              ( gToken.at( i ).type == ENTER || gToken.at( i ).type == COMMENT ) ) {
      i++ ;
      if ( isRightChild ) {
        BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // if
      else if ( isLeftChild ) {
        BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // else if
      else {
        BuildTree( tree, i, false, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ;        
      } // else 
    } // else if
    else if ( i < gToken.size() && gToken.at( i ).type == RIGHT_PAREN ) { // 笿珹腹
      RP++ ; // 珹腹计 + 1
      lastIndexOfToken = i ;
      if ( isLeftChild ) {
        i++ ;
        BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // if
      else {
        // i++ ;
        // BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // else  
    } // else if 笿珹腹
    else {
      if ( IsATOM( gToken.at( i ).type ) ) { // 笿ATOM
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->token = gToken.at( i ) ;
          tree->str = gToken.at( i ).str ;
          tree->left = NULL ;
          tree->right = NULL ; 
        } // if  
        else if ( isLeftChild ) { // 琌オ
          parent = tree->str ; 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;
        } // if オ
        else if ( isRightChild ) { // 琌
          parent = tree->str ;
          if ( parent == "." && gToken.at( i ).type == NIL ) { // 璝 ".nil" 玥ぃ翴(硂柑滦籠
            tree->str = "MimiNote:DontNeedToPutTwoSpace" ;     
          } // if 

          tree->right = new Node() ;
          tree = tree->right ;
          tree->left = NULL ;
          tree->right = NULL ;
          if ( parent == "." ) { // ΤDOT碞钡
            tree->str = gToken.at( i ).str ;
            tree->token = gToken.at( i ) ;
          } // if 
          else { // ⊿DOT ->NODE
            tree->str = "" ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕オnode
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ㄓ┕ 
          } // else         
        } // else if 

        lastIndexOfToken = i ;
      } // else if  笿ATOM
      else if ( gToken.at( i ).type == DOT ) { // 笿翴
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ; 
        } // if
        else {
          tree->str = "." ;
          tree->token = gToken.at( i ) ;
          i++ ; // ┕ǐ
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
          lastIndexOfToken = i ;
          int index = i + 1 ;
          bool toBreak = false ;
          while ( gToken.at( index ).str == "\n" ) { // 传︽常铬筁 
            i = index ;
            lastIndexOfToken = index ;
            index++ ;
          } // while 

          if ( index < gToken.size() && toBreak == false ) { // PEEKINDEX
            if ( gToken.at( index ).str == ")" && toBreak == false ) {
              RP++ ;
              lastIndexOfToken = index ;
              i = index ;             
            } // if
            else {
              toBreak = true ;
            } // else
          } // if 

        } // else       
      
      } // else if 笿翴
      else if ( gToken.at( i ).type == QUOTE ) { // 笿QUOTE

        if ( isLeftChild ) { // 穦琌オ 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = "quote" ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;

        } // if 

      } // else if 笿QUOTE
      else if ( gToken.at( i ).type == LEFT_PAREN ) { // 笿オ珹腹
        // bool rightChildDone = true ;
        LP++ ; // 衡オ珹腹计秖
        bool done = false ;
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ; 

          // 璶琌オ珹腹 常硂妓暗(ぇ ┕オǐ ┕)
          if ( done == false ) {
            tree->str = "(" ;
            tree->left = NULL ;
            tree->right = NULL ;
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕オǐ
            lastIndexOfToken = i ;
          } // if

          i++ ;
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕ǐ
          lastIndexOfToken = i ;
        } // if 
        else {
 
          parent = tree->str ;
          if ( parent == "." ) { // ﹚琌 琌ぃ惠璶NODE
            tree->str = "MimiNote:NeedToPutTwoSpace" ; // 璝 ".(" 玥ぃ翴(硂柑滦籠
            done = true ; 
            // 璶琌オ珹腹 常硂妓暗(ぇ ┕オǐ ┕)
            if ( done == false ) {
              tree->str = "(" ;
              tree->left = NULL ;
              tree->right = NULL ;
              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕オǐ
              lastIndexOfToken = i ;
            } // if

            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕ǐ
            lastIndexOfToken = i ;


          } // if 
          else { // parentぃ琌翴
            if ( isRightChild ) { // 璶暗˙(node)
              parent = tree->str ;
              tree->right = new Node() ;
              tree = tree->right ;
              tree->str = "" ;
              tree->left = NULL ;
              tree->right = NULL ;


              // 璶琌オ珹腹 常硂妓暗(ぇ ┕オǐ ┕)
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                // i++ ;
                parent = tree->str ; 
                // if ( i + 1 < gToken.size() && gToken.at( i+1 ).type == QUOTE ) i++ ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕オǐ
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕ǐ
              lastIndexOfToken = i ;


            } // if 
            else { // オ 

              parent = tree->str ;
              tree->left = new Node() ; // オ常璶┕オ(if parentぃ琌翴)
              tree->right = NULL ;
              tree = tree->left ;
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                i++ ;
                parent = tree->str ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕オǐ
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ┕ǐ
              lastIndexOfToken = i ;
            } // else オ 
          } // else parentぃ琌翴         
        } // else 
      

      } // else if 笿オ珹腹
    } // else 
  } // BuildTree()

  int SortOutQuote( int limitIndex ) { // 璝Τquote 碞盢quote玡常珹腹 'a b = ('a ) b 
    int i = 0 ; // index of gToken
    stack<int> indexToInsertLP ; // 础LPよ 
    Token gtempRP ; // RP
    gtempRP.str = ")" ; 
    gtempRP.error = false ;
    gtempRP.type = RIGHT_PAREN ;
    Token gtempLP ; // LP
    gtempLP.str = "(" ;
    gtempLP.error = false ;
    gtempLP.type = LEFT_PAREN ; 
    int count = 0 ; // 璸衡ぶ狥﹁
 
    while ( i < gToken.size() && i <= limitIndex ) {
      if ( gToken.at( i ).type == QUOTE && gToken.at( i ).str == "'" ) { // quoteよ碞琌础LPよ 
        indexToInsertLP.push( i ) ;
        i++ ;
        if ( i >= gToken.size() || i > limitIndex ) {
          ;
        } // if
        else { 
          while ( i < gToken.size() && gToken.at( i ).type == ENTER ) i++ ;
        } // else  

        if ( i >= gToken.size() ) {
          ;
        } // if 
        else if ( IsATOM( gToken.at( i ).type ) ) { // quote琌ATOM 
          int j = i + 1 ;
          gtempRP.lastIndexOfLine = gToken.at( i ).lastIndexOfLine ;
          if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
          else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RPATOM 
          count++ ; 
        } // else if quote琌ATOM
        else if ( gToken.at( i ).type == LEFT_PAREN ) { // quote琌オ珹腹  
          int j = i + 1 ;
          int lp = 1 ; // オ珹腹计秖 
          int rp = 0 ; // 珹腹计秖 
          bool toBreak = false ;
          while ( j < gToken.size() && toBreak == false ) {
            if ( gToken.at( j ).type == RIGHT_PAREN ) { // RP
              rp++ ;
            } // if RP
            else if ( gToken.at( j ).type == LEFT_PAREN ) { // LP
              lp++ ;
            } // else if LP 

            if ( lp == rp ) {
              gtempRP.lastIndexOfLine = gToken.at( j ).lastIndexOfLine ;
              gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP(癸嘿)珹腹 
              count++ ;
              toBreak = true ;
            } // if 

            j++ ; 
          } // while 
        } // else if quote琌オ珹腹 
        else if ( gToken.at( i ).type == QUOTE ) { // 硈尿QUOTE 
          int k = i ;
          while ( k < gToken.size() && gToken.at( k ).type == QUOTE ) { // 禲ぃ琌QUOTE 
            k++ ; 
          } // while 

          if ( k >= gToken.size() || k > limitIndex ) {
            ;
          } // if
          else {

            while ( gToken.at( k ).type == ENTER ) k++ ;
          } // else 

          if ( k >= gToken.size() ) {
            ;
          } // if 
          else {
            if ( IsATOM( gToken.at( k ).type ) ) { // quote琌ATOM 
              int j = k + 1 ;
              gtempRP.lastIndexOfLine = gToken.at( j ).lastIndexOfLine ;
              if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
              else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RPATOM
              count++ ; 
            } // if quote琌ATOM
            else if ( gToken.at( k ).type == LEFT_PAREN ) { // quote琌オ珹腹  
              int j = k + 1 ;
              int lp = 1 ; // オ珹腹计秖 
              int rp = 0 ; // 珹腹计秖 
              bool toBreak = false ;
              while ( j < gToken.size() && toBreak == false ) {
                if ( gToken.at( j ).type == RIGHT_PAREN ) { // RP
                  rp++ ;
                } // if RP
                else if ( gToken.at( j ).type == LEFT_PAREN ) { // LP
                  lp++ ;
                } // else if LP 

                if ( lp == rp ) {
                  gtempRP.lastIndexOfLine = gToken.at( i ).lastIndexOfLine ;
                  gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP(癸嘿)珹腹
                  count++ ; 
                  toBreak = true ;
                } // if 
 
                j++ ; 
              } // while 
            } // else if quote琌オ珹腹 
          } // else 
        } // else if 硈尿QUOTE 

        i-- ; // QUOTE(单穦i++) 
      } // if quoteよ碞琌础LPよ (Τ笿quote)

      i++ ; 
    } // while

    while ( ! indexToInsertLP.empty() ) { // 秨﹍insert LP(quoteindex ) 
      int index = indexToInsertLP.top() ;
      gtempLP.lastIndexOfLine = gToken.at( index ).lastIndexOfLine ;
      gToken.insert( gToken.begin()+index, gtempLP ) ;
      count++ ;
      indexToInsertLP.pop() ;
    } // while 

    return count ;
  } // SortOutQuote()


  void ReCalc( int index, int j ) { // // 眖材index秨﹍–lastIndexOfLine搭 (j+1)
    while ( index < gToken.size() && gToken.at( index ).str != "\n" ) {
      gToken.at( index ).lastIndexOfLine = gToken.at( index ).lastIndexOfLine - j - 1 ;
      index++ ;
    } // while 
  } // ReCalc()

  void DeleteGToken( int lastIndexOfToken ) { // 奔攫gToken
    int i = 0 ;
    int lastIndexOfLine = 0 ; 
    if ( lastIndexOfToken == -1 ) ;
    else if ( lastIndexOfToken >= gToken.size() ) { // 睲
      gToken.clear() ; 
    } // else if 
    else { // 场だ睲
      lastIndexOfLine = gToken.at( lastIndexOfToken ).lastIndexOfLine ;
      while ( ! gToken.empty() && i <= lastIndexOfToken ) {

        gToken.erase( gToken.begin() ) ; // 奔材ぇ穦患干ㄓ┮ッ环材
        i++ ;
      } // while 

      i = 0 ;
      if ( gToken.size() > 0 && lastIndexOfToken != -1 && gToken.at( i ).str == "\n" ) {
        lastIndexOfLine = -1 ;
        gToken.erase( gToken.begin() ) ;
      } // if
    } // else

    if ( gToken.size() > 0 ) 
      ReCalc( 0, lastIndexOfLine ) ;

  } // DeleteGToken()

  // ======================================================================================================= 

  bool IsProcedureFn( Token token ) {
    if ( token.funcType == BEEN_QUOTE || token.type == T ||  token.type == NIL ) return false ;
    if ( token.funcType == CONS || token.funcType == QUOTE_FN || token.funcType == DEFINE ||
         token.funcType == PART_ACCESSOR || token.funcType == PRIMITIVE_PREDICATE || 
         token.funcType == BEGIN_FN || token.funcType == OPERATOR_FN || token.funcType == EQU_TEST || 
         token.funcType == COND_FN || token.funcType == CLEAN_ENVIRONMENT || token.funcType == EXIT || 
         token.funcType == LET || token.funcType == LAMBDA ) {
      return true ;
    } // if

    return false ;
  } // IsProcedureFn()


  void Print( NodePtr tree, bool isLeftChild, bool isRightChild, int lp, int rp, string &parent,
              bool error ) { // –狥﹁ 碞 埃オ珹腹 ㄤ传︽

    if ( tree == NULL ) { // 娩琌NULL (オ娩ぃ琌NULL) 
      rp++ ;
      int times = lp - rp ;
      while ( times > 0 ) {
        cout << "  " ; // ㄢ
        times-- ;
      } // while

      if ( parent != "OMGISCOMMENT" ) {
        cout << ")\n" ;
        parent = ")" ;
      } // if 
    } // if 
    else if ( tree->left == NULL && tree->right == NULL ) { // ǐ┏
      int times = 0 ;
      bool hasPrintSpace = false ;   
      if ( isRightChild && tree->token.type == NIL ) { // ǐ┏セ琌NULL
        ; 
      } // if 
      else if ( tree->token.type == NIL ) {
        if ( parent != "(" ) {
          times = lp - rp ;
          while ( times > 0 ) {  
            cout << "  " ;
            times-- ;
          } // while 
        } // if 

        cout << "nil\n" ;
        parent = "nil" ;
      } // else if 
      else { // ǐ┏ セ琌ㄤ狥﹁
        if ( parent == "(" ) { // 璝琌オ珹腹 碞ぃノフ
          ;
        } // if
        else { // ぃ琌オ珹腹 ┮璶フ      
          times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // ㄢ
            times-- ; 
          } // while
        } // else   

        if ( error == false && IsProcedureFn( tree->token ) ) {
          string s = "#<procedure " + tree->str + ">" ;
          cout << s << "\n" ;
        } // if 
        else if ( tree->token.type == FLOAT ) {
          char str[100] = {"\0"} ;
          int i = 0 ;
          while ( i < tree->str.length() ) {
            str[i] = tree->str[i] ;
            i++ ;
          } // while

          float f = atof( str ) ;
          printf( "%.3f\n", f ) ;
        } // else if
        else if ( tree->token.type == INT ) {
          int ans ;
          stringstream ss ;
          ss << tree->str;
          ss >> ans ;
          
          
          // int ans = atoi( tree->str.c_str() ) ;
          printf( "%d\n", ans ) ;
          // cout << ans << "\n" ;
        } // else if INT
        else if ( tree->str != "" && tree->str != "(" && tree->token.type != NIL ) {
          cout << tree->str << "\n" ;
        } // else if

        parent = tree->str ; 
      } // else 

      if ( isRightChild ) { // 娩ǐ┏ Τ笿珹腹
        int times = lp - 1 ;
        while ( times > 0 ) {  
          cout << "  " ;
          times-- ;
        } // while 

        cout << ")\n" ;
        parent = ")" ;
        rp++ ;
      } // if 
    } // else if ǐ┏
    else { 
      bool dontNeedSpace = false ; 
      if ( isLeftChild ) { // ﹟ゼǐ┏ オ娩Node
        // if ( parent != "(" && parent != "" ) {
        if ( parent != "(" ) {
          int times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // ㄢ
            times-- ;
          } // while
        } // if 

        cout << "( " ;
        lp++ ;
        parent = "(" ; 
      } // if 


      Print( tree->left, true, false, lp, rp, parent, error ) ; // オ

      if ( ( tree->str != "." && tree->str != "" && tree->str != "(" 
           )
           || tree->right == NULL
           || tree->str == "MimiNote:DontNeedToPutTwoSpace"
           || tree->right->token.type == NIL ) 
        dontNeedSpace = true ;
                           
      if (  dontNeedSpace || parent == "(" || parent == "" || tree->str == "" ) { 
      // 璝琌オ珹腹 碞ぃノフ  
        ;
      } // if
      else if ( tree->str == "MimiNote:NeedToPutTwoSpace" ) {
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // ㄢ
          times-- ;
        } // while
      } // else if 
      else { // ぃ琌オ珹腹 ┮璶フ      
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // ㄢ
          times-- ;
        } // while
        
        
      } // else   

      if ( tree->str != "(" && tree->str != "" && tree->str != "MimiNote:DontNeedToPutTwoSpace" 
           && tree->str != "MimiNote:NeedToPutTwoSpace" ) // 
        cout << tree->str << "\n" ;

      parent = tree->str ;
      Print( tree->right, false, true, lp, rp, parent, error ) ; // 
    } // else
  } // Print()
  // ------------------------------------------------------------------------------------------- 
  
  bool IsSExp( NodePtr cur ) { // 耞cur攫琌S-Expression 
    if ( cur == NULL ) return true ; 
    else if ( cur != NULL && IsATOM( cur->token.type ) ) { // Atom
      return true ; 
    } // else if ATOM
    else if ( cur != NULL && cur->token.type == QUOTE ) { // QUOTE <S-exp>
      if ( IsSExp( cur->right ) == false ) { // 琵index禲赣挡よ
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( cur != NULL && ( cur->token.type == LEFT_PAREN || cur->token.type == DOT ) ) { 
      // オ珹腹 <S-exp>{<S-exp>}[DOT<S-exp>]珹腹
      if ( IsSExp( cur->left ) == false ) { // 耞オ攫 
        return false ;
      } // if 

      if ( IsSExp( cur->right ) == false ) { // 耞攫 
        return false ;
      } // if

      return true ;
    } // else if オ珹腹


    return false ;
  } // IsSExp() 

  // -------------------------------------------------------------------------------------------------------

  void PrettyPrint( NodePtr tree, bool error ) {
    bool isLeftChild = true ;
    bool isRightChild = false ;
    int lp = 0 ;
    int rp = 0 ;
    string parent = "OMGISCOMMENT" ;
    if ( error == false && tree != NULL && tree->left != NULL && 
         tree->left->token.funcType == LAMBDA ) {
      tree->str = "#<procedure lambda>" ;
      tree->left = NULL ;
      tree->right = NULL ;
    } // if 

    Print( tree, isLeftChild, isRightChild, lp, rp, parent, error ) ;
  } // PrettyPrint()

  // 耞琌﹚竡筁Symbol
  bool IsBoundSymbol( Token token ) {  
    if ( IsBoundFunction( token ) ) return true ;
    int i = 0 ;
    while ( i < gSymbolTAB.size() ) {
      if ( token.str == gSymbolTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while

    i = 0 ;
    while ( i < glocal.size() ) {
      if ( token.str == glocal.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while 

    if ( token.funcType == BEEN_QUOTE ) return true ;
    if ( token.funcType == CUSTOMIZE ) return true ;
    return false ;     
  } // IsBoundSymbol()

  // 璝List玥娩ǐ┏琌NULL 
  bool IsList( NodePtr tree ) {
    NodePtr pre = tree ;
    while ( tree != NULL ) {   
      pre = tree ;  
      tree = tree->right ;
    } // while 

    if ( pre->token.type == NIL ) return true ;
    else if ( IsATOM( pre->token.type ) ) return false ; // 琌纒癌ぃ琌ATOM    
    else return true ;
  } // IsList() 

  // 耞琌﹚竡筁Function
  bool IsBoundFunction( Token token ) {  
    // ρ砏﹚function 
    if ( token.funcType == CONS || token.funcType == QUOTE_FN || token.funcType == DEFINE ||
         token.funcType == PART_ACCESSOR || token.funcType == PRIMITIVE_PREDICATE || 
         token.funcType == BEGIN_FN || token.funcType == OPERATOR_FN || token.funcType == EQU_TEST || 
         token.funcType == COND_FN || token.funcType == CLEAN_ENVIRONMENT || token.funcType == EXIT ||
         token.funcType == LET || token.funcType == LAMBDA ) {
      return true ;
    } // if
    else if ( token.str == "exit" ) { // ぃ絋﹚癸ぃ癸 硂妓糶 
      return true ;
    } // else if 
    
    // ㄤ﹚竡Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( token.str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;     
  } // IsBoundFunction()

  bool IsTopLevel( NodePtr cur ) { // 耞琌ぃ琌top Level 
    NodePtr temp = gTree->left ;
    if ( temp == cur ) {
      return true ;
    } // if 
    
    return false ;
  } // IsTopLevel()

  bool IsDoubleTon( NodePtr cur ) {
    if ( IsATOM( cur->token.type ) ) {
      return false ;
    } // if 
    else {
      cur = cur->right ;
      if ( cur == NULL || cur->token.type == NIL ) {
        return false ;
      } // if 
      
      while ( cur != NULL && cur->token.type != NIL ) {
        if ( cur->left != NULL && cur->right != NULL && IsATOM( cur->right->token.type ) ) {
          if ( cur->right->token.type != NIL ) return false ;
 
        } // if 
        
        cur = cur->right ;
      } // while 
    } // else 
    
    return true ;
  } // IsDoubleTon()

  bool IsPair( NodePtr cur ) { // ( ( x 5 ) ( y '(1 2 3)) ) LET耞 
    while ( cur != NULL && cur->token.type != NIL ) {
      if ( cur->left != NULL && ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) ) {
        return false ;
      } // if 
      else { // ( x 5 ) 耞珹腹柑 
        NodePtr temp = cur->left ; // temp琌オ珹腹  
        if ( temp->left != NULL && temp->left->token.type != SYMBOL ) { // 莱赣璶琌Symbol 
          return false ;
        } // if  

        temp = temp->right ;
        if ( temp == NULL ) return false ; // ⊿Τ﹚竡 玥 ERROR 
        if ( temp->left != NULL && IsSExp( temp->left ) == false ) { 
          // 耞よ琌ぃ琌sexp (莱SExp)
          return false ;
        } // if

        temp = temp->right ;
        if ( temp != NULL && temp->token.type != NIL ) {
          return false ;
        } // if   
      } // else 

      cur = cur->right ;
    } // while 

    return true ;
  } // IsPair() 
  
  bool CheckFormat( FunctionType type, NodePtr cur, string & errormsg ) { // DEFINE COND_FN LET
    if ( type == DEFINE ) { // DEFINE
      int numOfArgs = CountNumOfArgs( cur ) ;
      if ( numOfArgs != 2 && numOfArgs < 2 ) {
        errormsg = "ERROR (DEFINE format) : " ;
        return false ;
      } // if 
      else {
        if ( cur->left->token.type == SYMBOL ) { // SYMBOL
          if ( numOfArgs != 2 ) {
            errormsg = "ERROR (DEFINE format) : " ;
            return false ;
          } // if 

          if ( cur->left->token.type != SYMBOL ) { // cur = (1)
            errormsg = "ERROR (DEFINE format) : " ;
            return false ;
          } // if
          else {  
            if ( IsProcedureFn( cur->left->token ) ) {
              errormsg = "ERROR (DEFINE format) : " ;
              return false ;  
            } // if 
          
                
            cur = cur->right ;
            bool isSExp = IsSExp( cur->left ) ;
          
            if ( isSExp == false ) {
              errormsg = "ERROR (DEFINE format) : " ;
              return false ;
            } // if 
            else {
              cur = cur->right ;
              if ( cur != NULL ) { // (define a 10 20 ) Τ把计 ex: ( define a 10 ) 
                if ( cur->token.type == NIL ) {
                  return true ;
                } // if 

                errormsg = "ERROR (DEFINE format) : " ;
                return false ;
              } // if 
            
              return true ; // 秈evaluation() 
            } // else 
          } // else 
        } // if Symbol
        else { // ( define ( F x ) ( ... ) ) 
          if ( IsATOM( cur->left->token.type ) ) {
            errormsg = "ERROR (DEFINE format) : " ;
            return false ;
          } // if 

          NodePtr bone = cur ;
          cur = cur->left ;
          bool first = true ;
          while ( cur != NULL && cur->token.type != NIL ) { // 耞( F x y ) 琌ぃ琌常琌symbol 
            if ( IsATOM( cur->left->token.type ) == false ) { 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 
            else if ( cur->left->token.type != SYMBOL ) { // 獶symbol ( F 3 ) 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // else if

            if ( first && IsProcedureFn( cur->left->token ) ) {  // ぃ﹚竡ρ﹚竡function 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 

            cur = cur->right ;
          } // while 

          bone = bone->right ;
          
          while ( cur != NULL && cur->token.type != NIL ) { // 1~ S-Exp 
            cur = bone->left ; 
            if ( IsSExp( cur->left ) == false ) return false ;
            cur = cur->right ;
          } // while 

          return true ;
        } // else 
      } // else 
        
    } // if DEFINE
    else if ( type == COND_FN ) { // COND
      bool isSExp = true ;
      bool isDoubleTon = true ; 
      int numOfArgs = CountNumOfArgs( cur ) ;
      if ( numOfArgs < 1 ) {
        isSExp = false ;
      } // if

      while ( isSExp && isDoubleTon && cur != NULL && cur->token.type != NIL ) {        
        isSExp = IsSExp( cur->left ) ;      
        isDoubleTon = IsDoubleTon( cur->left ) ;  
        cur = cur->right ; // // 传纒癌(把计)浪琩ㄤオ攫琌琌<S-exp>     
      } // while 
      
      if ( ( cur == NULL || cur->token.type == NIL ) && isSExp && isDoubleTon ) { // ゅ猭タ絋 
        return true ;
      } // if 
      else { // ゅ猭岿粇 
        errormsg = "ERROR (COND format) : " ;
        return false ;
      } // else 
    } // else if
    else if ( type == LET ) { // LET
      int numOfArgs = CountNumOfArgs( cur ) ; // cur 眖材把计纒癌秨﹍ 
      if ( numOfArgs != 2 && numOfArgs < 2 ) { // 把计场だΤㄢ纒癌 (ぃ衡let场だ) 
        errormsg = "ERROR (LET format) : " ;
        return false ;
      } // if 
      else {
        if ( IsATOM( cur->left->token.type ) && cur->left->token.type == NIL ) ;
        else if ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) { 
          // 莱赣璶琌node ぃ琌ATOM (莱赣璶琌オ珹腹) 
          errormsg = "ERROR (LET format) : " ;
          return false ;
        } // else if
        else { // 材把计琌オ珹腹秨繷 (zero-or-more-PAIRs)  ( ( x 5 ) ( y '(1 2 3))) 
          if ( IsPair( cur->left ) == false ) { // 肚硂秈耞 ( ( x 5 ) ( y '(1 2 3)) )
            errormsg = "ERROR (LET format) : " ;
            return false ;
          } // if 
        } // else 
      } // else 
    } // else if LET 
    

    return true ;
  } // CheckFormat()

  int CountNumOfArgs( NodePtr cur ) { // 衡argument计秖 
    if ( cur != NULL && cur->left != NULL ) {
      return CountNumOfArgs( cur->right ) + 1 ;
    } // if
    else {
      return 0 ;
    } // else 
  } // CountNumOfArgs()

  void CheckNumOfArgs( FunctionType type, NodePtr in, int numOfArgs, bool &error, string & errormsg ) {
    // lambdaerrormsg璶矪瞶 (FNい矪瞶) 
    if ( type == CONS ) {
      if ( in->left->str == "cons" && numOfArgs != 2 ) { // cons(2)
        error = true ;  
      } // if 把计计秖岿粇
      else if ( in->left->str == "list" && numOfArgs < 0 ) { // list(>=0) 
        error = true ;  
      } // else if 
      else {
        error = false ;
      } // else 
    } // if
    else if ( type == QUOTE_FN ) { // quote(1)
      if ( numOfArgs != 1 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == PART_ACCESSOR ) { // car(1), cdr(1)
      if ( numOfArgs != 1 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if 
    else if ( type == PRIMITIVE_PREDICATE ) { // 常Τ1 : atom? pair? null? integer?... 
      if ( numOfArgs != 1 ) { // 把计计秖岿粇
        error = true ;   
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == OPERATOR_FN ) { // +-*/ > = < not and or...
      if ( in->left->str == "not" && numOfArgs != 1 ) { // not(1)
        error = true ;           
      } // if 把计计秖岿粇
      else if ( in->left->str != "not" && numOfArgs < 2 ) { // ㄤ常璶 >= 2 
        error = true ;
      } // else if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EQU_TEST ) { //   eqv?(2) equal?(2)
      if ( numOfArgs != 2 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == BEGIN_FN ) { // begin
      if ( numOfArgs < 1 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "cond" && numOfArgs < 1 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "if" && ( numOfArgs != 2 || numOfArgs != 3 ) ) { // 把计计秖岿粇
        error = true ;   
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CLEAN_ENVIRONMENT ) { // clear_env.
      if ( numOfArgs != 0 ) { // 把计计秖岿粇
        error = true ;  
      } // if 把计计秖岿粇
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CUSTOMIZE ) { // ﹚竡FN 
      Fn f ;
      int i = 0 ;
      bool found = false ;
      while ( i < gFunctionTAB.size() && !found ) { // 眖gFunctionTABいтFN 
        if ( in->left->str == gFunctionTAB.at( i ).str ) { // тFN 
          f = gFunctionTAB.at( i ) ;
          found = true ;
        } // if
        
        i++ ; 
      } // while
      
      if ( numOfArgs != f.numOfArgs ) { // 耞把计计秖癸ぃ癸 
        error = true ;        
      } // if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EXIT ) {
      if ( numOfArgs != 0 ) { // 耞把计计秖癸ぃ癸 
        error = true ;        
      } // if
      else error = false ; 
    } // else if 

    if ( error ) {
      errormsg = "ERROR (incorrect number of arguments) : " + in->left->str + "\n" ; 
    } // if error
  } // CheckNumOfArgs()

  bool DoNotNeedEvalARGS( NodePtr in ) { // DEFINE/ QUOTE/ COND/ if/ and/ or ぃ惠璶耞args ┮璶铬筁

    FunctionType fnType = in->token.funcType ;
    string str = in->token.str ;
    if ( fnType == QUOTE_FN || fnType == DEFINE || fnType == COND_FN || fnType == CUSTOMIZE || 
         str == "if" || str == "and" || str == "or" || fnType == LET || fnType == LAMBDA ) {
      return true ;
    } // if

    return false ;
  } // DoNotNeedEvalARGS()

  NodePtr GetDefineSymbolNode( NodePtr node, bool useLocal ) {
    int i = 0 ;
    if ( useLocal ) { // symbol琌local  
      while ( i < glocal.size() ) {
        if ( glocal.at( i ).str == node->token.str ) {
          return glocal.at( i ).value ;
        } // if 
      
        i++ ;
      } // while 
    } // if 

    // 璝local⊿т 
    i = 0 ;
    while ( i < gSymbolTAB.size() ) {
      if ( gSymbolTAB.at( i ).str == node->token.str ) {
        return gSymbolTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 

    // SYMBOL ⊿т этfunction 
    i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( gFunctionTAB.at( i ).str == node->token.str ) {
        return gFunctionTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 
        
    return node ;
  } // GetDefineSymbolNode()   
  
  bool IsCustomize( string str ) { // 琌﹚竡function 
    // ㄤ﹚竡Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;
  } // IsCustomize() 

  bool CheckCustomizeArgs( string str, NodePtr in ) { // 耞﹚竡fn把计琌 
     // ㄤ﹚竡Function 
    int i = 0 ;
    int numOfArgs = 0 ; 
    int index = 0 ; // 赣﹚竡function竚 
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) {
      if ( str == gFunctionTAB.at( i ).str ) { // т 
        toBreak = true ;
        index = i ;
      } // if 
      
      i++ ;
    } // while

    i = 0 ;
    while ( in != NULL && in->token.type != NIL ) { // 璸衡(攫)Τぶ把计 
      numOfArgs++ ;
      in = in->right ;
    } // while 

    if ( numOfArgs == gFunctionTAB.at( index ).numOfArgs ) return true ;
    else return false ;  
  } // CheckCustomizeArgs()

  void DefineLocal( string funcName, NodePtr in, bool useLocal, bool & error, string & errormsg, 
                    NodePtr & errorNode ) { 
    // ﹚竡local variable 玥function暗ㄆ 
    // (肚function把计(癬﹍)┮bone) 
    int i = 0 ;
    bool toBreak = false ;
    bool find = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // тfunctionт璶﹚竡把计 
      if ( gFunctionTAB.at( i ).str == funcName ) {
        toBreak = true ;
      } // if 
      else {
        i++ ;
      } // else 
    } // while 

    Fn f = gFunctionTAB.at( i ) ;
    Symbol s ; 
    NodePtr out = NULL ;
    NodePtr in_temp = in ; // 魁in秨﹍竚 
    // 盢┮Τ璶砆﹚竡璸衡Ч拨 
    while ( in != NULL && in->token.type != NIL && error == false ) { 
      Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        errormsg = "ERROR (unbound symbol) : " ;
        errorNode = in->left ;
      } // if 

      in->left = out ; // 把计 
      in = in->right ; // 传把计 
    } // while 

    // 盢璶砆﹚竡把计倒ぉ 
    i = 0 ;
    in = in_temp ; 
    while ( error == false && i < f.args.size() ) { // 盢把计﹚竡 
      s.str = f.args.at( i ) ; // 把计嘿
      s.value = in->left ; // 把计
      if ( s.value != NULL && IsBoundSymbol( s.value->token ) ) {
        DeleteDefineSym( s.str ) ;
      } // if 

      glocal.push_back( s ) ; // glocalい 
      in = in->right ; // 传把计 
      i++ ;
    } // while 

  } // DefineLocal()

  NodePtr GetFnDefine( string fnName ) { // 眔fnName硂function璶磅︽ㄆ 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // тfunctionт璶﹚竡把计 
      if ( gFunctionTAB.at( i ).str == fnName ) {
        toBreak = true ;
      } // if 
      else {
        i++ ;
      } // else 
    } // while

    NodePtr f = NULL ;
    NodePtr val = gFunctionTAB.at( i ).value ;
    CopyTree( f, val ) ;
    return f ; 
  } // GetFnDefine()

  bool IsProcedureFn_Str( string str, FunctionType & fnType ) { // 璝string琌ρ﹚竡finction 肚type 
    if ( str == "cons" || str == "list" ) {
      fnType = CONS ;
      return true ;
    } // if 
    else if ( str == "quote" || str == "\'" ) {
      fnType = QUOTE_FN ;
      return true ;
    } // else if 
    else if ( str == "define" ) {
      fnType = DEFINE ;
      return true ;
    } // else if 
    else if ( str == "car" ||  str == "cdr" ) {
      fnType = PART_ACCESSOR ;
      return true ;
    } // else if
    else if ( str == "atom?" || str == "pair?" || str == "list?" || str == "null?" || str == "integer?" || 
              str == "real?" || str == "number?" || str == "string?" || str == "boolean?" || 
              str == "symbol?" ) {
      fnType = PRIMITIVE_PREDICATE ;
      return true ;
    } // else if
    else if ( str == "+" || str == "-" || str == "*" || str == "/" || str == "and" || str == "or" || 
              str == "not"  ) {
      fnType = OPERATOR_FN ;
      return true ;
    } // else if
    else if ( str == ">" || str == ">=" || str == "<" || str == "<=" || str == "=" || 
              str == "string-append" || str == "string>?" || str == "string<?" ) {
      fnType = OPERATOR_FN ;
      return true ;
    } // else if
    else if ( str == "eqv?" || str == "equal?" ) {
      fnType = EQU_TEST ;
      return true ;
    } // else if
    else if ( str == "begin" ) {
      fnType = BEGIN_FN ;
      return true ;
    } // else if
    else if ( str == "if" || str == "cond" ) {
      fnType = COND_FN ;
      return true ;
    } // else if
    else if ( str == "clean-environment" ) {
      fnType = CLEAN_ENVIRONMENT ;
      return true ;
    } // else if
    else if ( str == "exit" ) {
      fnType = EXIT ;
      return true ;
    } // else if 
    else {
      return false ;
    } // else 
  } // IsProcedureFn_Str()

  bool CheckLambdaFormat( NodePtr in ) { // 浪琩lambdaformat琌才 
    // (lambda (zero-or-more-symbolsぃ琌constant) one-or-more-S-expressions )

    NodePtr bone = in->right ; // 把计┮bone 
    if ( bone == NULL || bone->token.type == NIL ) return false ; // ( lambda ) 
    in = in->right ;
    // 浪琩 (zero-or-more-symbolsぃ琌constant)
    if ( IsATOM( in->left->token.type ) && in->left->token.type == NIL ) ; // () 箂symbol
    else if ( IsATOM( in->left->token.type ) ) return false ; // ( lambda 5 )
    else { // 材把计 1~symbol()ぃ琌计ê摸琌symbol 
      in = in->left ;
      while ( in != NULL && in->token.type != NIL ) {
        if ( IsATOM( in->left->token.type ) == false ) return false ;
        else if ( in->left->token.type != SYMBOL ) return false ;
        in = in->right ; 
      } // while 
    } // else 

    // 浪琩 one-or-more-S-expressions
    bone = bone->right ; // 材把计┮bone   
    if ( bone == NULL || bone->token.type == NIL ) return false ; // ﹚璶1~ S-Exp 
    while ( bone != NULL && bone->token.type != NIL ) {
      in = bone->left ; // 材把计┮竚
      if ( IsSExp( in ) == false ) return false ;
      bone = bone->right ;
    } // while 

    return true ;
  } // CheckLambdaFormat()

  void CheckArgsType( NodePtr in, NodePtr para, bool & error, string & errormsg ) { // in = (root)
    // check para type琌骸ìin->str 硂function 
    string str = "" ; // fnName 

    if ( in != NULL && in->left != NULL )
      str = in->left->str ;

    if ( str == "car" || str == "cdr" ) {
      if ( IsPair( in, errormsg ) == false ) error = true ; 
    } // if car cdr
    else if ( str == "let" ) {
      if ( IsPair( in ) == false ) {
        error = true ;
        errormsg = "ERROR (" + str + " with incorrect argument type) : " ;
      } // if 
    } // else if 
    else if ( str == "+" || str == "-" || str == "*" || str == "/" || str == ">" || str == ">=" || 
              str == "<" || str == "<=" || str == "=" ) {
      if ( para->token.type != INT && para->token.type != FLOAT ) {
        error = true ;
        errormsg = "ERROR (" + str + " with incorrect argument type) : " ;
      } // if          	
    } // else if 
    else if ( str == "string>?" || str == "string<?" || str == "string=?" || str == "string-append" ) {
      if ( para->token.type != STRING ) {
        error = true ;
        errormsg = "ERROR (" + str + " with incorrect argument type) : " ;
      } // if 
    } // else if 

  } // CheckArgsType() // ( fn, 把计)	

  void GetBoundSymbol( NodePtr in, NodePtr & out, bool useLocal ) {
    bool find = false ;
    if ( IsBoundFunction( in->token ) ) { // 琌砆﹚竡筁function 
      out = new Node() ;   
      string originName = GetOriginFn( in->str ) ;  
      out->str = originName ; // out->str = "#<procedure " + originName + ">" ;
      out->token = in->token ;
      FunctionType fnType = NONE ;
      if ( IsProcedureFn_Str( out->str, fnType ) == false ) out->token.funcType = CUSTOMIZE ;
        
      if ( IsProcedureFn_Str( out->str, fnType ) == false ) {
        out->str = "#<procedure " + originName + ">" ;
      } // if
 
      out->left = NULL ;
      out->right = NULL ;
      find = true ;
    } // if 琌砆﹚竡筁function
    else {
      out = in ; 
      int i = 0 ;
      if ( useLocal ) { // 琩т跋办跑计
                    
        while ( i < glocal.size() ) {
          if ( in->token.str == glocal.at( i ).str ) { 
            out = glocal.at( i ).value ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // 琌lambdatop level 玥<procedure lambda > 
            } // if 

            find = true ;
          } // if 
      
          i++ ;
        } // while

      } // if 

      i = 0 ;
      if ( find == false ) { // 琩т办跑计 
        while ( i < gSymbolTAB.size() && find == false ) {
          if ( in->token.str == gSymbolTAB.at( i ).str ) { 
            out = gSymbolTAB.at( i ).value ;
            FunctionType fnType = NONE ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // 琌lambdatop level 玥<procedure lambda > 
            } // if 
            else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
              // 埃ρ﹚竡function蛤 "砆quote" top level function (﹚竡function)
              out->token.funcType = fnType ;
            } // else if 

            find = true ; 
          } // if 
      
          i++ ;
        } // while
      } // if  т办跑计    
    } // else

    if ( find == false ) { // ⊿﹚竡symbol 
      out = in ;
    } // if
  } // GetBoundSymbol()
  
  void Eval( NodePtr in, NodePtr & out, bool & error, string & errormsg, NodePtr &errorNode, 
             bool useLocal ) { // useLocal = 琌ㄏノlocal variable 
    bool done = false ; // define/ cond/ if/ and / or钡暗Ч碞return挡狦 
    bool customizeDone = false ;
    vector<Symbol> glocalTemp ; // 侣glocalfn挡ぇ
    NodePtr copy_tree = NULL ;
    CopyTree( copy_tree, in ); 
    
    if ( IsATOM( in->token.type ) && in->token.type != SYMBOL ) { // ATOM 
      out = in ; // return that atom
      out->left = NULL ;
      out->right = NULL ;
    } // if ATOM
    else if ( in->token.type == SYMBOL ) { // SYMBOL 
      bool find = false ;
      if ( IsBoundFunction( in->token ) ) { // 琌砆﹚竡筁function 
        out = new Node() ;   
        string originName = GetOriginFn( in->str ) ;  
        out->str = originName ; // out->str = "#<procedure " + originName + ">" ;
        out->token = in->token ;
        FunctionType fnType = NONE ;
        if ( IsProcedureFn_Str( out->str, fnType ) == false ) out->token.funcType = CUSTOMIZE ;
        
        
        if ( IsTopLevel( in->left ) && IsProcedureFn_Str( out->str, fnType ) == false ) {
          out->str = "#<procedure " + originName + ">" ;
        } // if
 
        out->left = NULL ;
        out->right = NULL ;
        find = true ;
      } // if 琌砆﹚竡筁function
      else {
        out = in ; 
        int i = 0 ;
        if ( useLocal ) { // 琩т跋办跑计
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // 琌lambdatop level 玥<procedure lambda > 
              } // if 

              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if 

        i = 0 ;
        if ( find == false ) { // 琩т办跑计 
          while ( i < gSymbolTAB.size() && find == false ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
              FunctionType fnType = NONE ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // 琌lambdatop level 玥<procedure lambda > 
              } // if 
              else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
                // 埃ρ﹚竡function蛤 "砆quote" top level function (﹚竡function)
                out->token.funcType = fnType ;
              } // else if 

              find = true ; 
            } // if 
      
            i++ ;
          } // while
        } // if  т办跑计    
      } // else

      if ( find == false ) { // ⊿﹚竡symbol 
        out = in ;
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->str + "\n" ;
      } // if
    } // else if SYMBOL
    else { // node ( root )
      if ( IsList( in ) == false ) { // is not a (pure) list
        error = true ;
        errormsg = "ERROR (non-list) : " ;
        errorNode = in ;
      } // if is not a (pure) list
      else if ( IsBoundSymbol( in->left->token ) == false && IsATOM( in->left->token.type ) && 
                in->left->token.type != SYMBOL ) 
      { // ATOM(non-fn)
        error = true ;
        errormsg = "ERROR (attempt to apply non-function) : " ;
        errorNode = in->left ;
      } // else if 
      else if ( in->left->token.type == SYMBOL || in->left->token.type == QUOTE ) { // first arg is SYMBOL
        if ( IsBoundFunction( in->left->token ) ) { // 琌﹚竡筁Function // 
           
          // ぃ琌TopLevel define clear env.------------------------------------------(A-START)
          // PART_A
          bool isTopLevel = IsTopLevel( in->left ) ;
         
          if ( isTopLevel == false  &&  in->left->token.funcType == DEFINE ) { // (A-1) 
            error = true ;
            errormsg = "ERROR (level of DEFINE)\n" ;        
          } // if ぃ琌TopLevel define
          else if ( isTopLevel == false  && in->left->token.funcType == CLEAN_ENVIRONMENT ) {
            error = true ;
            errormsg = "ERROR (level of CLEAN-ENVIRONMENT)\n" ;
          } // else if ぃ琌TopLevel clear_env. 
          else if ( isTopLevel == false  && in->left->token.str == "exit" ) {
            error = true ;
            errormsg = "ERROR (level of EXIT)\n" ;
          } // else if ぃ琌TopLevel exit.  -----------------------------------------------------(A-1)
          else if ( isTopLevel && in->left->token.funcType == LAMBDA ) { // ( lambda XXXXXX )  
            out = in->left ;
            if ( CheckLambdaFormat( in ) == false ) {
              error = true ;    
              errormsg = "ERROR (lambda format) : " ;
              errorNode = in ;
              out = NULL ;
            } // if 
          } // else if ( lambda XXXXXX )  
          else if ( in->left->token.funcType == DEFINE || in->left->token.str == "cond" || 
                    in->left->token.funcType == LET ) { // (A-2)
            bool isformatCurrect = CheckFormat( in->left->token.funcType, in->right, errormsg ) ;
            if ( isformatCurrect == false ) { // ERROR
              errorNode = in ; // error岿êNode : (define ) 肚程珹腹() 
              error = true ;
            } // if
            else { // formatタ絋璸衡俱tree(眖in秨﹍)
              NodePtr temp = NULL ; 
              out = temp ;
            } // else 
          } // else if DEFINE COND_FN --------------------------------------------------------------(A-2)
          else if ( in->left->token.str == "if" || in->left->token.str == "and" || 
                    in->left->token.str == "or" ) { // "if" "and" "or ----------------------------(A-3)
            int numOfArgs = CountNumOfArgs( in->right ) ;   
            if ( in->left->token.str == "if" ) { // "if"
              if ( numOfArgs != 2 && numOfArgs != 3 ) {
                error = true ;
                errormsg = "ERROR (incorrect number of arguments) : if\n" ;
              } // if
              else {
                ; 
              } // else 
            } // if
            else { // "and" "or
              if ( numOfArgs < 2 ) {
                error = true ;
                errormsg = "ERROR (incorrect number of arguments) : " + in->left->str + "\n" ;
              } // if
              else {
                ;
              } // else 
            } // else      
          } // else if "if" "and" "or --------------------------------------------------------------(A-3)
          else { // --------------------------------------------------------------------------------(A-4)
            if ( IsCustomize( in->left->str ) ) { // ﹚竡function 
              if ( CheckCustomizeArgs( in->left->str, in->right ) == false ) {
                error = true ;
                errormsg = "ERROR (incorrect number of arguments) : " ;
                bool temp_error = false ;
                Eval( in->left, errorNode, temp_error, errormsg, errorNode, useLocal ) ; 
              } // if 
              else { // 把计计秖タ絋 
                in->left->token.funcType = CUSTOMIZE ;
                glocalTemp = glocal ;
                // 肚function把计(癬﹍)┮bone
                DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
                NodePtr fnDefine = GetFnDefine( in->left->str ) ; // 眔function璶暗ㄆ 
                useLocal = true ;
                
                // eval( Second arg S2 of the main S-exp)
                NodePtr in_args = fnDefine ; // in_argsオ娩Τ把计纒癌  in main_S-exp 
                NodePtr out_result = NULL ;
                bool paraCorrect = true ; 
                if ( error ) { // 把计琌Τ 
                   paraCorrect = false ;
                } // if 

                while ( in_args != NULL && in_args->token.type != NIL && paraCorrect ) { 
                  // nodeleftΤ把计
	              if ( error && errormsg == "ERROR (unbound parameter) : " ) {
                    error = false ;
                    errormsg = "" ;	
	              } // if 

                  Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = 把计
                  in_args->left = out_result ; // 钡癬ㄓ
                  if ( error == false ) {
                    CheckArgsType( in, out_result, error, errormsg ) ; // ( fn(token), 把计)	
                    if ( error )  {
                      errorNode = out_result ;
                      if ( IsBoundSymbol( out_result->token ) ) {      
                        NodePtr temp = NULL ;      
                        bool temp_error = false ;   
                        GetBoundSymbol( out_result, errorNode, useLocal ) ;
                      } // if
                    } // if 
                  } // if 

                  if ( error && errormsg == "ERROR (no return value) : " ) {
                    errormsg = "ERROR (unbound parameter) : " ;
                  } // if

                  in_args = in_args->right ; // 传纒癌 
                } // while

                out = out_result ; 
                if ( paraCorrect && out == NULL ) {
                  error = true ;
                  errormsg = "ERROR (no return value) : " ;
                  errorNode = copy_tree ;
                } // if 
                /*
                if ( error == false ) { // 礚error膥尿暗 
                  Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // 磅︽function
                  if ( error && errormsg == "ERROR (no return value) : " ) errorNode = copy_tree ;
                } // if 
                */               
                customizeDone = true ;
              } // else 
            } // if 
            else { // 獶﹚竡function: car cdr 
              int numOfArgs = CountNumOfArgs( in->right ) ;
              // Check numof args 琌タ絋 
              CheckNumOfArgs( in->left->token.funcType, in, numOfArgs, error, errormsg ) ;
            } // else 
                          
          } // else --------------------------------------------------------------------------------(A-4)
          
          // ---------------------------------------------------------------------------------------(A-END)
        } // if 琌﹚竡筁Function 
        else { // SYMBOLぃ琌砆﹚竡筁function嘿 
          if ( IsBoundSymbol( in->left->token ) == false ) { // SYMBOL﹟ゼ砆﹚竡
            error = true ; 
            errormsg = "ERROR (unbound symbol) : " + in->left->str + "\n" ;
          } // if 
          else { // ﹚竡SYMBOLぃ琌function
             
            NodePtr define_node = GetDefineSymbolNode( in->left, useLocal ) ;
            NodePtr temp_node = NULL ;
            CopyTree( temp_node, define_node ) ; 
            in->left = temp_node ;
            if ( IsProcedureFn( define_node->token ) ) { // 琌﹚竡筁"ρ﹚FN" 钡癬ㄓ ┮ぃノ暗ㄆ 
              ; // in->left->token = define_node->token ; // in->left->token = define_token ;            
            } // if 
            else if ( in->left != NULL && in->left->left != NULL && 
                      in->left->left->token.funcType == LAMBDA ) {
              useLocal = true ;
            } // else if 
            else if ( IsBoundFunction( in->left->token ) ) {
              useLocal = true ;
            } // else if 
            else {
              NodePtr temp = NULL ; 
              error = true ;
              errormsg = "ERROR (attempt to apply non-function) : " ;
              errorNode = define_node ;  
            } // else 
               
          } // else 
        } // else  SYMBOLぃ琌砆﹚竡筁function嘿 
      } // else if first argument is SYMBOL
      else { // ( () ... )  ()
        // evaluate () 碞琌in->left(0) 
        NodePtr temp = in ;
        NodePtr tempOut = NULL ;
        if ( in->left != NULL && in->left->left != NULL && 
             in->left->left->token.funcType == LAMBDA ) { // 璝lambda 
          useLocal = true ;
        } // if
        else { // 獶lambdaㄤ 
          Eval( in->left, tempOut, error, errormsg, errorNode, useLocal ) ;
        
          if ( error == false ) {
            in->left = tempOut ; // 钡癬ㄓ 
            // ----------------------------------------------------------------------- PART_B (START) 
            // 莱Fn嘿  璝tempセō琌ATOM 碞ぃ琌FN  
            if ( IsATOM( tempOut->token.type ) && IsBoundFunction( tempOut->token ) ) { 
              // ( IsATOM( tempOut->token.type ) == false && IsBoundFunction( tempOut->left->token ) ) 
              int numOfArgs = CountNumOfArgs( temp->right ) ; // in->rightFN把计纒癌1 
              // Check numof args 琌タ絋 
              CheckNumOfArgs( temp->left->token.funcType, temp, numOfArgs, error, errormsg ) ;
              // lambdaerrormsg璶矪瞶 ┪琌CheckNumOfArgsい矪瞶 
            } // if
            else if ( tempOut->left != NULL && tempOut->left->token.funcType == LAMBDA ) {
              // ( lambda (...) (...) )
              /*
              if ( CheckLambdaFormat( in ) == false ) {
                error = true ;
                errormsg = "ERROR (lambda format) : " ;
                errorNode = tempOut ;
                out = NULL ;
              } // if
              */
            } // else if 
            else { // 獶Fn嘿 
              error = true ;
              errormsg = "ERROR (attempt to apply non-function) : " ;
              // errormsg = errormsg + tempOut->str + "\n" ;
              errorNode = tempOut ;  
            } // else
          
            // ---------------------------------------------------------------------------(PART_B) END 
          } // if
        } // else 獶lambdaㄤ
          
        
      } // else ( () ... )  ()
      
      
      NodePtr out_result = NULL ;
      if ( DoNotNeedEvalARGS( in->left ) || in->left->token.funcType == LAMBDA ) { 
        // Τㄇぃ惠璶耞把计璶钡秈︽笲衡 
        ; 
      } // if
      else if ( in->left != NULL && in->left->left != NULL && 
                in->left->left->token.funcType == LAMBDA ) {
        useLocal = true ;
      } // else if 
      else { // 埃QUOTE常璶膥尿耞把计 
        // eval( Second arg S2 of the main S-exp)
        NodePtr in_args = in->right ; // in_argsオ娩Τ把计纒癌  in main_S-exp 
        while ( in_args != NULL && in_args->token.type != NIL ) { // nodeleftΤ把计
	      if ( error && errormsg == "ERROR (unbound parameter) : " ) {
            error = false ;
            errormsg = "" ;	
	      } // if 

          Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = 把计
          in_args->left = out_result ; // 钡癬ㄓ
          if ( error == false ) {
            CheckArgsType( in, out_result, error, errormsg ) ; // ( fn(token), 把计)	
            if ( error )  {
              errorNode = out_result ;
              if ( IsBoundSymbol( out_result->token ) ) {      
                NodePtr temp = NULL ;      
                bool temp_error = false ;   
                GetBoundSymbol( out_result, errorNode, useLocal ) ;
              } // if
            } // if 
          } // if 

          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            errormsg = "ERROR (unbound parameter) : " ;
          } // if

          in_args = in_args->right ; // 传纒癌 
        } // while
      } // else 
        

      if ( error == false && customizeDone == false ) { 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // 璸衡挡狦肚
        out = out_result ;
      } // if
      else if ( error && errormsg == "ERROR (unbound parameter) : " ) {
        error = false ;
        errormsg = "" ; 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // 璸衡挡狦肚
        out = out_result ; 
      } // else if 
      else { // ﹚竡磅︽Ч 
        glocal = glocalTemp ; // 盢侣glocalㄓ 碞琌盢local variables pop奔
      } // else 

      // ---------------------------------------------------------------------------(PART_C) END  
    } // else 
  } // Eval()

  bool IsPair( NodePtr cur, string & errormsg ) { // (cur = root)

    NodePtr fn = cur->left ; // fn 既() car, cdr硂ㄇfunctionNode (errormsg惠璶fnノ)
    cur = cur->right ; // (1)
    if ( cur->right != NULL && cur->right->token.type != NIL ) { // ex: (car 3 4) error (Τ把计) 
      errormsg = "ERROR (incorrect number of arguments) : " + fn->str + "\n" ;        
      return false ;
    } // if  ex: (car 3 4) error
    
    // Token define_token = GetDefineToken( cur->left->token ) ; 
    if ( IsATOM( cur->left->token.type ) ) { // ( car 3 ) cur->left 碞琌 3 硂node 
      errormsg = "ERROR (" + fn->str + " with incorrect argument type) : " ;
      return false ;
    } // if ( car 3 ) cur->left 碞琌 3 硂node 
    
    return true ; 
  } // IsPair()

  bool HasExistInSymbolTAB( string str, int & index ) {
    int i = 0 ;
    while ( i < gSymbolTAB.size() ) {
      if ( str == gSymbolTAB.at( i ).str ) {
        index = i ;
        return true ;
      } // if
      
      i++ ; 
    } // while
    
    return false ; 
  } // HasExistInSymbolTAB()

  void BeenQuoted( NodePtr cur ) { // 盢cur┮攫把计 埃疭種竡(砆quote癬ㄓ種)
    if ( cur == NULL ) {
      ;
    } // if 
    else if ( cur->left == NULL && cur->right == NULL ) {
      cur->token.funcType = BEEN_QUOTE ;
    } // else if
    else {
      BeenQuoted( cur->left ) ;
      BeenQuoted( cur->right ) ;
    } // else 
     
  } // BeenQuoted()

  void FloatStrToIntStr( string in, string & out ) { // 盢float锣int(stringよΑ纗) 
    int i = 0 ;
    bool toBreak = false ;
    out = "" ;
    while ( i < in.size() && toBreak == false ) {
      if ( in[i] == '.' ) {
        toBreak = true ;
      } // if 
      else {
        out = out + in[i] ;
      } // else  

      i++ ;
    } // while 
  } // FloatStrToIntStr()

  void Add( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr & errorNode ) {
    // 盢cur┮攫┮Τ"把计" 
    float sum = 0 ;
    float num = 0 ; // ノㄓ钡把计float(string)
    bool isInt = true ; // 魁琌float猭临琌integer猭 
    result = new Node() ; // 肚穝Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // 锣Θfloat笲衡
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // typeぃ琌intぃ琌float 
        error = true ;
        result = NULL ;
        errormsg = "ERROR (+ with incorrect argument type) : " ;
        errorNode = cur->left ;
      } // if
      else {
        sum = sum + num ;
        cur = cur->right ;
      } // else 
         
    } // while 

    if ( error == false ) {
    // 璝琌3.0 璶int 碞穦钡3
    // 璝琌璶float "%.3f"穦琌癸氮
      string ans_str = "" ;
      stringstream ss ;
      ss << sum ;
      ss >> ans_str ; // result->str ;    

      result->str = ans_str ;   
      if ( isInt ) { // Integer
        result->token.type = INT ;
        string str_out = "" ;
        FloatStrToIntStr( result->str, str_out ) ;
        result->str = str_out ;
      } // if
      else { // Float
        result->token.type = FLOAT ;
      } // else 
    } // if     
  } // Add() 

  void Sub( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr & errorNode ) {
    // 盢cur┮攫┮Τ"把计" 
    bool isInt = true ; // 魁琌float猭临琌integer猭 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计typeぃタ絋 
      error = true ;
      errormsg = "ERROR (- with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // 材把计typeタ絋 
      float sum = atof( cur->left->str.c_str() ) ; // 材把计 
      if ( cur->left->token.type == FLOAT ) { // 耞材把计type 
        isInt = false ;
      } // if 
      
      float num = 0 ; // ノㄓ钡把计float(string)    
      result = new Node() ; // 肚穝Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // 传把计 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        num = atof( cur->left->str.c_str() ) ; // 锣Θfloat笲衡
        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // typeぃ琌intぃ琌float 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (- with incorrect argument type) : " ;
          errorNode = cur->left ;
        } // else if
        else {
          sum = sum - num ;
          cur = cur->right ;
        } // else 
           
      } // while 
  
      if ( error == false ) {
      // 璝琌3.0 璶int 碞穦钡3
      // 璝琌璶float "%.3f"穦琌癸氮 
        stringstream ss ;
        ss << sum ;
        ss >> result->str ;    
        
        if ( isInt ) { // Integer
          result->token.type = INT ;
          string str_out = "" ;
          FloatStrToIntStr( result->str, str_out ) ;
          result->str = str_out ;
        } // if
        else { // Float
          result->token.type = FLOAT ;
        } // else 
      } // if     
    } // else     
  } // Sub()

  void Multi( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr & errorNode ) { 
    // 盢cur┮攫┮Τ"把计" 
    float sum = 1 ;
    float num = 0 ; // ノㄓ钡把计float(string)
    bool isInt = true ; // 魁琌float猭临琌integer猭 
    result = new Node() ; // 肚穝Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // 锣Θfloat笲衡
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // typeぃ琌intぃ琌float 
        error = true ;
        result = NULL ;
        errormsg = "ERROR (* with incorrect argument type) : " ;
        errorNode = cur->left ;
      } // if
      else {
        sum = sum * num ;
        cur = cur->right ;
      } // else 
         
    } // while 

    if ( error == false ) {
    // 璝琌3.0 璶int 碞穦钡3
    // 璝琌璶float "%.3f"穦琌癸氮 
      stringstream ss ;
      ss << sum ;
      ss >> result->str ;    
      
      if ( isInt ) { // Integer
        result->token.type = INT ;
        string str_out = "" ;
        FloatStrToIntStr( result->str, str_out ) ;
        result->str = str_out ;
      } // if
      else { // Float
        result->token.type = FLOAT ;
      } // else 
    } // if     
  } // Multi()

  void Div( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr & errorNode ) {
    // 盢cur┮攫┮Τ"把计" 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计typeぃタ絋 
      error = true ;
      errormsg = "ERROR (/ with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // 材把计typeタ絋 
      bool isInt = true ; // 魁琌float猭临琌integer猭
      // float sum = atof( cur->left->str.c_str() ) ; // 材把计 
      double sum ;
      stringstream ss ;
      ss << cur->left->str ;
      ss >> sum ;
      
      
      
      // double sum = atof( cur->left->str.c_str() ) ;
      if ( cur->left->token.type == FLOAT ) { // 耞材把计type 
        isInt = false ;
      } // if 
      
      // float num = 0 ; // ノㄓ钡把计float(string) 
      double num = 0 ;
      result = new Node() ; // 肚穝Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // 传把计 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        // num = atof( cur->left->str.c_str() ) ; // 锣Θfloat笲衡

        stringstream ss ;
        ss << cur->left->str ;
        ss >> num ;



        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // typeぃ琌intぃ琌float 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (/ with incorrect argument type) : " ;
          errorNode = cur->left ;
        } // if
        else {
          if ( num == 0 ) {
            error = true ;
            result = NULL ;
            errormsg = "ERROR (division by zero) : /\n" ;  
          } // if 
          else {
            sum = sum / num ;
            cur = cur->right ;
          } // else              
        } // else 
           
      } // while 
  
      if ( error == false ) {
      // 璝琌3.0 璶int 碞穦钡3
      // 璝琌璶float "%.3f"穦琌癸氮 
        stringstream ss ;
        ss << sum ;
        ss >> result->str ;    
        
        if ( isInt ) { // Integer
          result->token.type = INT ;
          string str_out = "" ;
          FloatStrToIntStr( result->str, str_out ) ;
          result->str = str_out ;
        } // if
        else { // Float
          result->token.type = FLOAT ;
        } // else 
      } // if     
    } // else     
  } // Div()

  void And( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode, 
            bool useLocal ) { // And
    NodePtr lastSExp = NULL ;
    bool toBreak = false ;
    NodePtr temp = NULL ;
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // 纒癌セぃ琌NIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ; // 铬癹伴 
      } // if 
      else if ( temp->token.type == NIL ) { // т材NIL 碞肚NIL 
        result = temp ;
        toBreak = true ; // 铬癹伴 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // 俱狥﹁常⊿ΤNIL 
      result = lastSExp ; // 玥肚程S-Exp 
    } // if 
  } // And()

  void Or( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal ) { // Or
    NodePtr lastSExp = NULL ;
    bool toBreak = false ;
    NodePtr temp = NULL ;
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // 纒癌セぃ琌NIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ;
      } // if 
      else if ( temp->token.type != NIL ) {
        result = temp ;
        toBreak = true ; // 铬癹伴 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // 俱狥﹁常琌NIL 
      result = lastSExp ; // 玥肚程S-Exp (碞琌NIL) 
    } // if 
  } // Or()

  void BiggerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f, 
                   NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (> with incorrect argument type) : " ; 
      errorNode = cur->left ;     
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 材把计琌计 
      float f2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (> with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          f2 = atof( cur->left->str.c_str() ) ; // 把计琌计 
          if ( f1 > f2 ) {
            f1 = f2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ;
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // BiggerThan()

  void BiggerEquThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, 
                      NodePtr f, NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (>= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 材把计琌计 
      float f2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (>= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          f2 = atof( cur->left->str.c_str() ) ; // 把计琌计 
          if ( f1 >= f2 ) {
            f1 = f2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ;
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // BiggerEquThan()

  void SmallerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f,
                    NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (< with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 材把计琌计 
      float f2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (< with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          f2 = atof( cur->left->str.c_str() ) ; // 把计琌计 
          if ( f1 < f2 ) {
            f1 = f2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ;
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // SmallerThan()

  void SmallerEquThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, 
                       NodePtr f, NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (<= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 材把计琌计 
      float f2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (<= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          f2 = atof( cur->left->str.c_str() ) ; // 把计琌计 
          if ( f1 <= f2 ) {
            f1 = f2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ;
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // SmallerEquThan()

  void Equ( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f,
            NodePtr & errorNode ) {
    bool toBreak = false ;
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 材把计琌计 
      float f2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          f2 = atof( cur->left->str.c_str() ) ; // 把计琌计 
          if ( f1 == f2 ) {
            f1 = f2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ;
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // Equ()



  void StrBiggerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, 
                      NodePtr f, NodePtr & errorNode ) {
    bool toBreak = false ;
    bool isTrue = true ; 
    
    if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string>? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 材把计琌计 
      string s2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string>? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          s2 = cur->left->str ; // 把计琌计 
          if ( s1 > s2 ) {
            s1 = s2 ;
          } // if 
          else {  
            isTrue = false ;            
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
      else if ( isTrue == false ) {
        result = f ;
      } // else if 
    } // else 
  } // StrBiggerThan()

  void StrSmallerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, 
                       NodePtr f, NodePtr & errorNode ) {
    bool toBreak = false ;
    bool isTrue = true ;  
    if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string<? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 材把计琌计 
      string s2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string<? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          s2 = cur->left->str ; // 把计琌计 
          if ( s1 < s2 ) {
            s1 = s2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ; 
          } // else
          
          cur = cur->right ; 
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // StrSmallerThan()

  void StrEqu( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f,
               NodePtr & errorNode ) {
    bool toBreak = false ;
    bool isTrue = true ;  
    if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string=? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 材把计琌计 
      string s2 ;
      cur = cur->right ; // 材把计 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 材把计ぃ琌计 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string=? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 把计琌计
          s2 = cur->left->str ; // 把计琌计 
          if ( s1 == s2 ) {
            s1 = s2 ;
          } // if 
          else {              
            result = f ;
            isTrue = false ; 
          } // else 
          
          cur = cur->right ;
        } // else 
      } // while 
      
      if ( toBreak == false && isTrue ) { // return true
        result = t ;
      } // if 
    } // else 
  } // StrEqu()

  void StrAppend( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr & errorNode ) {
    bool toBreak = false ;
    result = new Node() ;
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    result->token.type = STRING ;
    result->str = "" ; // init
    string str = "" ;
    string temp = "" ; // ぇstring 
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {
      if ( cur->left->token.type != STRING ) {
        error = true ;
        result = NULL ;
        errormsg = "ERROR (string-append with incorrect argument type) : " ;
        errorNode = cur->left ;
        toBreak = true ;
      } // if 
      else {
        temp = cur->left->str ; // "a"
        temp = temp.substr( 1, temp.length()-2 ) ;
        str = str + temp ;
      } // else
      
      cur = cur->right ; 
    } // while 
    
    if ( error == false ) {
      result->str = "\"" + str + "\"" ;
    } // if 
  } // StrAppend()

  void Eqv( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f ) {
    NodePtr para1 = cur->left ; // 材把计
    cur = cur->right ;
    bool isTrue = false ;
    NodePtr para2 = cur->left ; // 材把计 
    if ( para1 == para2 ) {
      isTrue = true ;
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type ) ) { // 常琌ATOM 
      if ( para1->str == para2->str ) { // 
        if ( para1->token.type == STRING && para2->token.type == STRING ) { // 璝琌String玥ㄒ -> false 
          result = f ;  
        } // if 
        else {
          isTrue = true ;
        } // else 
      } // if 
      else { // 常琌ATOM(str)ぃ 
        result = f ;
      } // else 
    } // else if 
    else {
      result = f ;
    } // else

    if ( isTrue ) {
      if ( IsProcedureFn( para1->token ) && IsProcedureFn( para2->token ) ) {
        result = t ;
      } // if
      else if ( IsProcedureFn( para1->token ) == false && IsProcedureFn( para2->token ) == false ) {
        result = t ;
      } // else if
      else result = f ; 
    } // if  
  } // Eqv()
  
  NodePtr Equal( NodePtr para1, NodePtr para2, NodePtr t, NodePtr f ) {
    bool isTrue = false ;
    if ( para1 == NULL || para2 == NULL ) {  
      if ( para1 == NULL && ( para2 == NULL || para2->token.type == NIL ) ) {
        return t ;
      } // if 
      else if ( para2 == NULL && ( para1 == NULL || para1->token.type == NIL ) ) {
        return t ;
      } // else if 
      else { // ㄤい词攫挡 词﹟ゼ挡 
        return f ;
      } // else 
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type )  ) { // 常琌ATOM 
      if ( para1->str != para2->str ) {
        return f ;
      } // if
      else {
        // return t ;
        isTrue = true ;
      } // else 
    } // else if
    else if ( IsATOM( para1->token.type ) && para2->token.funcType == BEEN_QUOTE && para2->left == NULL ) {
      if ( para1->str != para2->str ) {
        return f ;
      } // if
      else {
        isTrue = true ;
      } // else
    } // else if 
    else if ( IsATOM( para2->token.type ) && para1->token.funcType == BEEN_QUOTE && para1->left == NULL ) {
      if ( para1->str != para2->str ) {
        return f ;
      } // if
      else {
        isTrue = true ;
      } // else
    } // else if
    else if ( IsATOM( para1->token.type ) == false && IsATOM( para2->token.type ) == false ) { // 常ぃ琌ATOM
      NodePtr status = Equal( para1->left, para2->left, t, f ) ;
      if ( status->str == "#t" ) {
        return Equal( para1->right, para2->right, t, f ) ;
      } // if 
      else {
        return f ;
      } // else  
    } // else if  常ぃ琌ATOM
    else { // ㄤい琌ATOM ぃ琌 
      return f ;
    } // else ㄤい琌ATOM ぃ琌

    if ( isTrue ) {
      if ( IsProcedureFn( para1->token ) && IsProcedureFn( para2->token ) ) {
        return t ;
      } // if
      else if ( IsProcedureFn( para1->token ) == false && IsProcedureFn( para2->token ) == false ) {
        return t ;
      } // else if
      else return f ; 
    } // if
    else return f ; 
  } // Equal()

  void CopyTree( NodePtr &temp, NodePtr cur ) { // temp = cur
    
    if ( cur == NULL ) {
      ;
    } // if
    else {
      if ( temp == NULL ) {
        temp = new Node() ;
        temp->str = cur->str ;
        temp->token = cur->token ;
        temp->left = NULL ;
        temp->right = NULL ; 
      } // if 

      
      CopyTree( temp->left, cur->left ) ; // ┕オǐ 
      CopyTree( temp->right, cur->right ) ; // ┕ǐ 
    } // else
    
  } // CopyTree()

  void If( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal, NodePtr copy_tree ) {
    NodePtr temp = NULL ;
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // рcur硂攫copytempい 
    NodePtr tempOut = NULL ;
    temp = temp->right ; // 材把计 (璸衡Ч莱False(nil) ┪ 獶False)
    Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
    // (璸衡Ч莱False(nil) ┪ 獶False)
    if ( error == false ) {
      temp = temp->right ;
      if ( tempOut->token.type != NIL ) { // true
        Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
        if ( error == false ) {
          result = tempOut ;
        } // if no error
      } // if 
      else { // false
        temp = temp->right ; // 材把计┮纒癌 
        if ( temp == NULL || temp->token.type == NIL ) {
          error = true ;
          errormsg = "ERROR (no return value) : " ;
          errorNode = copy_tree ;
        } // if
        else {
          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
          if ( error == false ) {
            result = tempOut ;
          } // if no error
        } // else 
      } // else 
    } // if no error
      
  } // If()

  void Cond( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
             bool useLocal, NodePtr copy_tree ) {
    NodePtr temp = NULL ;
    NodePtr bone = NULL ; // 璽砫ǐ纒癌场だ 
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // рcur硂攫copytempい  
    NodePtr tempOut = NULL ;
    temp = temp->right ; // 材把计 (材if阀├)
    bone = temp ; // 材纒癌
    bool done = false ;
    while ( error == false && done == false && bone != NULL && bone->token.type != NIL ) {
      temp = bone->left ; // if/ else if / else 竚
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // (璸衡Ч莱False(nil) ┪ 獶False)
      
      if ( error && ( bone->right == NULL || bone->right->token.type == NIL ) 
           && tempOut != NULL && tempOut->str == "else" ) {
        // 程纒癌 琌else   ( ㄤelse 砆﹚Θ unbound symbol ) 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          error = false ; // 砞false 膥尿衡
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 
 
          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 璶肚挡狦 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error        

          temp = temp->right ;
        } // while 
      } // if 程纒癌
      else if ( error == false && ( bone->right == NULL || bone->right->token.type == NIL ) 
                && temp->left->str == "else" ) { 
        // else 砆﹚竡筁 else 琌程Sexp 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 

          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 璶肚挡狦 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error

          temp = temp->right ;
        } // while 
      } // else if else 砆﹚竡筁 else 琌程Sexp  
      
    
      if ( done == false && error == false ) {

        if ( tempOut->token.type != NIL ) { // true
          temp = temp->right ;
          while ( temp != NULL && temp->token.type != NIL && done == false ) {
            if ( error == true && errormsg == "ERROR (no return value) : " ) {
              error = false ;
              errormsg = "" ;
            } // if 

            Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 璶肚挡狦 
            if ( error == false ) {
              result = tempOut ;
            } // if no error
            
            temp = temp->right ;
          } // while 
          
          if ( ( temp == NULL || temp->token.type == NIL ) && error == false ) {
            done = true ;
          } // if       
        } // if
        else {
          bone = bone->right ; 
        } // else 
      } // if        
    } // while
    
    if ( done == false && error == false ) {
      error = true ;
      errormsg = "ERROR (no return value) : " ;
      errorNode = copy_tree ;
    } // if     
  } // Cond()

  void DefineSym( NodePtr bone ) { // ﹚竡local variable 
    NodePtr temp = bone->left ;
    NodePtr tempOut = NULL ;
    bool error = false ; 
    string errormsg = "" ;
    NodePtr errorNode = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr temp_bone = bone ; // 既﹍bone 
    while ( bone != NULL && bone->token.type != NIL ) { // bone琌程 ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp琌柑珹腹 ( x 5 ) ┪( y 3 ) 
      temp = temp->right ;
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 璸衡赣symbolvalue
      temp->left = tempOut ;
      bone = bone->right ; // 传把计 
    } // while 
 
    bone = temp_bone ;
    while ( error == false && bone != NULL && bone->token.type != NIL ) { 
      // bone琌程 ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp琌柑珹腹 ( x 5 ) ┪( y 3 ) 
      s.str = temp->left->str ; // symbol 
      temp = temp->right ;
      s.value = temp->left ;
      glocal.push_back( s ) ; // 盢local variablevectorい 
      bone = bone->right ; // 传把计 
    } // while 
  } // DefineSym()


  void DeleteDefineFn( string str ) { // ﹚竡function 
    // ㄤ﹚竡Function 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) {
      if ( str == gFunctionTAB.at( i ).str ) {
        gFunctionTAB.erase( gFunctionTAB.begin() + i ) ;
        toBreak = true ;
      } // if 
      
      i++ ;
    } // while
  } // DeleteDefineFn() 

  void DeleteDefineSym( string str ) { // ﹚竡symbol
    // ㄤ﹚竡Function 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gSymbolTAB.size() && toBreak == false ) {
      if ( str == gSymbolTAB.at( i ).str ) {
        gSymbolTAB.erase( gSymbolTAB.begin() + i ) ;
        toBreak = true ;
      } // if 
      
      i++ ;
    } // while
  } // DeleteDefineSym() 


  int GetFnArgsNum( string fnName, vector<string> & args ) { // тfnName硂function肚ㄤ把计戈癟 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) {
      if ( fnName == gFunctionTAB.at( i ).str ) {
        args = gFunctionTAB.at( i ).args ;
        return gFunctionTAB.at( i ).numOfArgs ;
      } // if 
      else {
        i++ ;
      } // else 
    } // while

    return 0 ;
  } // GetFnArgsNum()
 
  string GetOriginFn( string fnName ) { // тfnNameセ琌砆街﹚竡 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( fnName == gFunctionTAB.at( i ).str ) {
        if ( gFunctionTAB.at( i ).originFn == "" ) { // 琌ネfunction
          return fnName ;
        } // if 
        else { // 琌パ﹚竡 
          return gFunctionTAB.at( i ).originFn ; 
        } // else 
      } // if 

      i++ ;
    } // while

    return fnName ; 
  } // GetOriginFn()

  int GetProcedureFnArgs( NodePtr f ) { // тρ┮﹚竡function肚砏﹚把计计秖 
    if ( f->str == "cons" || f->str == "define" || f->token.funcType == OPERATOR_FN || f->str == "and" || 
         f->str == "or" || f->token.funcType == EQU_TEST ) return 2 ;

    return 1 ; 
  } // GetProcedureFnArgs() 


  void DefineLambdaLocal( NodePtr para1, NodePtr para2, bool & error, string & errormsg, 
                          NodePtr & errorNode ) { // ﹚竡lambda把计 
    int i = 0 ;

    NodePtr tempOut = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr p2 = para2 ; // 既para2秨﹍竚 
    
    // 场常璸衡璶砆﹚竡glocalい 
    // while 璽砫璸衡┮Τ璶砆﹚竡 
    while ( para2 != NULL && para2->token.type != NIL && error == false ) { 
      Eval( para2->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 璸衡赣symbolvalue
      para2->left = tempOut ;
      tempOut = NULL ;
      para2 = para2->right ;
    } // while 

    para2 = p2 ; 
    while ( error == false && para1 != NULL && para1->token.type != NIL && para2 != NULL && 
            para2->token.type != NIL ) {
      s.str = para1->left->str ;
      s.value = para2->left ;
      if ( IsBoundSymbol( s.value->token ) ) {
        DeleteDefineSym( s.str ) ;
      } // if 

      glocal.push_back( s ) ; 
      tempOut = NULL ;
      para1 = para1->right ;
      para2 = para2->right ;
    } // while 

    if ( para1 == NULL && para2 == NULL ) ;
    else if ( ( para1 == NULL ) && ( para2 != NULL ) ) {
      if ( para2->token.type != NIL ) {
        error = true ;
        errormsg = "ERROR (incorrect number of arguments) : " ;
      } // if 
      
    } // else if 
    else if ( ( para1 != NULL ) && ( para2 == NULL ) ) {
      error = true ;
      errormsg = "ERROR (incorrect number of arguments) : " ;
    } // else if 
  } // DefineLambdaLocal()

  void Evaluate( NodePtr in, NodePtr &out, bool & error, string & errormsg, NodePtr & errorNode,
                 bool useLocal, vector<Symbol> glocalTemp, NodePtr copy_tree ) {
    if ( IsATOM( in->token.type ) && in->token.type != SYMBOL ) {
      out = in ;
    } // if
    else if ( in->token.type == SYMBOL ) {
      if ( IsBoundSymbol( in->token ) ) {
        out = in ;
        bool find = false ;
        int i = 0 ;
        if ( useLocal ) { // 琩т跋办跑计
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if

        i = 0 ; 
        if ( find == false ) { // 琩т办跑计 
          while ( i < gSymbolTAB.size() ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
            } // if 
      
            i++ ;
          } // while
        } // if
      } // if
      else { // 獶﹚竡symbol 
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->token.str + "\n" ;
      } // else 
    } // else if 
    else if ( in->left->token.funcType == CONS ) { // CONS (cons, list)  
      if ( in->left->token.str == "cons" ) { // cons
        in = in->right ;
        NodePtr temp = new Node() ;
        temp->left = in->left ; // 材把计 オ攫
        in = in->right ;
        temp->right = in->left ; // 材把计 攫
        if ( IsATOM( in->left->token.type ) && in->left->token.type != NIL ) { // 璝钡ATOM 璶DOT 
          temp->str = "." ; // い丁癘眔翴 
          temp->token.type = DOT ;
        } // if 
        else { // 璝琌LIST 碞ぃ璶DOT 
          temp->str = "(" ; // い丁癘眔翴 
          temp->token.type = LEFT_PAREN ;
        } // else
 
        out = temp ;
      } // if cons  
      else if ( in->left->token.str == "list" ) { // list
        out = in->right ;
        if ( out == NULL ) {
          out = new Node() ;
          out->left = NULL ;
          out->right = NULL ;
          out->str = "nil" ;
          out->token.type = NIL ;
        } // if 
      } // else if list        
    } // else if CONS (cons, list)
    else if ( in->left->token.funcType == QUOTE_FN ) { // QUOTE_FN
      in = in->right ; // (1)
      out = in->left ;
      BeenQuoted( out ) ; // 盢ㄤ┮攫跑Θ"" (碞琌ê狥﹁)/ 埃疭種竡 
    } // else if  QUOTE_FN
    else if ( in->left->token.funcType == DEFINE ) { // DEFINE
      out = new Node() ;
      out->left = NULL ;
      out->right = NULL ;
      in = in->right ;
      if ( in->left->token.type == SYMBOL ) { // is SYMBOL
        string symbol = in->left->str ; // symbol 
        out->str = symbol + " defined" ;
        NodePtr para1 = in->left ; // 材把计セ b 
        NodePtr para2 = in->right->left ; // 材把计セ a  
        in = in->right ;
        NodePtr temp = NULL ;
        if ( IsBoundFunction( para2->token ) ) { // ( define b a ) a琌﹚竡function 
          out->token.type = STRING ;
          out->token.funcType = CUSTOMIZE ; 
          Fn f ;
          if ( IsBoundFunction( para1->token ) ) { // 璝b砆﹚竡funciton玥奔穝﹚竡
            DeleteDefineFn( para1->str ) ; // 奔ぇ玡﹚竡function 
          } // if
          else if ( IsBoundSymbol( para1->token ) ) {
            DeleteDefineSym( para1->str ) ; // 奔ぇ玡﹚竡function 
          } // else if 

          f.str = para1->str ;
          if ( IsProcedureFn( para2->token ) ) { // 琌ρ﹚竡function 
            Symbol temps ;
            temps.str = f.str ;
            temps.value = para2 ;
            gSymbolTAB.push_back( temps ) ;

          } // if
          else { // 琌и﹚竡function 
            f.value = GetFnDefine( para2->str ) ;
            f.originFn = GetOriginFn( para2->str ) ; // т硂琌パfunction┮﹚竡 
            f.numOfArgs = GetFnArgsNum( para2->str, f.args ) ; 
            gFunctionTAB.push_back( f ) ;  // 盢﹚竡functiongFunctionTABい
          } // else 

                            
        } // if // ( define b a ) a琌﹚竡function 
        else { // 琌璶﹚竡symbol 
          Symbol s ;
          s.str = symbol ;
          int index = 0 ;
          if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
            s.value = in->left ;                      
          } // if 琌lambda 
          else { // ぃ琌lambda                     
            Eval( in->left, temp, error, errormsg, errorNode, useLocal ) ; // Symbol璶砆﹚竡
            out->token.type = STRING ;
            out->token.funcType = CUSTOMIZE ; 
            if ( error == false ) {          
              s.value = temp ;       
            } // if           
          } // else ぃ琌lambda 

          if ( HasExistInSymbolTAB( s.str, index ) ) { // 竒gSymbolTABい
            gSymbolTAB.insert( gSymbolTAB.begin() + index, s ) ; // 秈 
            gSymbolTAB.erase( gSymbolTAB.begin() + index + 1 ) ; // 奔セ 
          } // if 
          else { // ぇ玡⊿瞷筁 ┮础程 
            gSymbolTAB.push_back( s ) ; 
          } // else 
        } // else 琌璶﹚竡symbol 
      } // if SYMBOL
      else { // define (F x ) ( s-exp )
        NodePtr bone = in ;
        in = in->left ; // (F  x y z)
        Fn temp_Fn ;
        int counter = 0 ; // 璸衡把计计秖ノ 
        if ( IsBoundFunction( in->left->token ) ) { // 璝砆﹚竡funciton玥奔穝﹚竡 
          DeleteDefineFn( in->left->str ) ; // 奔ぇ玡﹚竡function 
        } // if 

        temp_Fn.str = in->left->str ; // Fn name
        out->str = temp_Fn.str + " defined" ;
        out->token.type = STRING ;
        out->token.funcType = CUSTOMIZE ;
        if ( gVerbose == false ) { // verbose琌false 碞ぃ璶 
          out = NULL ;
        } // if
 
        in = in->right ;
        while ( in != NULL && in->token.type != NIL ) { // 盢┮Τfunction把计常癬ㄓ 
          counter++ ;
          temp_Fn.args.push_back( in->left->str ) ;
          in = in->right ;
        } // while 

        temp_Fn.numOfArgs = counter ;
        in = bone->right ; // 传Fn﹚竡(璶暗ぐ或ㄆ)
        temp_Fn.value = in ;
        temp_Fn.value->token.funcType = CUSTOMIZE ;
        temp_Fn.originFn = temp_Fn.str ; // 琌ネfunction 
        gFunctionTAB.push_back( temp_Fn ) ; 
      } // else define (F x ) ( s-exp )
    } // else if DEFINE
    else if ( in->left->token.funcType == PART_ACCESSOR ) { // PART_ACCESSOR(car, cdr)
      if ( IsPair( in, errormsg ) ) {
        NodePtr temp = in->right->left ; // list竚(10)
        NodePtr tempOut = NULL ;
        if ( in->left->str == "car" ) {
          out = temp->left ; 
        } // if car
        else { // cdr
          out = temp->right ;
        } // else cdr

        if ( out == NULL ) {

          out = new Node() ;
          out->str = "nil" ;
          out->token.type = NIL ;
          out->left = NULL ;
          out->right = NULL ;
        } // if 
      } // if 
      else { // error (not pair)
        error = true ;
      } // else    
    } // else if PART_ACCESSOR(car, cdr)
    else if ( in->left->token.funcType == PRIMITIVE_PREDICATE ) { // PRIMITIVE_PREDICATE
      NodePtr t = new Node() ;
      t->str = "#t" ;
      t->token.type = T ; // 獶盽璶 ﹚璶砞﹚type
      t->left = NULL ;
      t->right = NULL ;
      NodePtr f = new Node() ;
      f->str = "nil" ;
      f->token.type = NIL ;
      f->left = NULL ;
      f->right = NULL ;
      NodePtr temp ;
      
      if ( in->left->token.str == "atom?" ) { // atom?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) ) { // true
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // if atom?
      else if ( in->left->token.str == "pair?" ) { // pair?
        if ( IsPair( in, errormsg ) ) {
          out = t ;
        } // if 
        else {
          out = f ;
        } // else 
      } // else if pair?
      else if ( in->left->token.str == "list?" ) { // list?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) {
          ;
        } // if error
        else if ( IsList( in->left ) ) { // true
          out = t ; 
        } // else if
        else { // false
          out = f ;
        } // else 
      } // else if list?
      else if ( in->left->token.str == "null?" ) { // null? 
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == NIL ) { // true
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // else if null?
      else if ( in->left->token.str == "integer?" ) { // integer?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == INT ) { // true
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // else if  integer?
      else if ( in->left->token.str == "real?" || in->left->token.str == "number?" ) { // real? number?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == INT || temp->token.type == FLOAT ) ) {
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false        
      } // else if real? number?
      else if ( in->left->token.str == "string?" ) { // string?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == STRING ) { // true
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // else if  string?
      else if ( in->left->token.str == "boolean?" ) { // boolean?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 把计Τ岿粇 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == T || temp->token.type == NIL ) ) {
          out = t ;
        } // else if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // else if  boolean?
      else if ( in->left->token.str == "symbol?"  ) { // symbol?
        in = in->right ; // (1)
        temp = in->left ;
        // if ( temp->token.type == SYMBOL || temp->token.funcType == BEEN_QUOTE ) { 
        // '3 ぃ琌symbol 
        if ( temp->token.type == SYMBOL ) {
          out = t ;
        } // if true
        else { // 礚岿粇false 
          out = f ;
        } // else false
      } // else if symbol?
    } // else if PRIMITIVE_PREDICATE
    else if ( in->left->token.funcType == OPERATOR_FN ) { // OPERATOR_FN(+-*/)
      NodePtr t = new Node() ;
      t->str = "#t" ;
      t->token.type = T ; // 獶盽璶 ﹚璶砞﹚type
      t->left = NULL ;
      t->right = NULL ;
      NodePtr f = new Node() ;
      f->str = "nil" ;
      f->token.type = NIL ;
      f->left = NULL ;
      f->right = NULL ;

      // Token define_token = GetDefineToken( in->left->token ) ;
       
      if ( in->left->str == "+" ) { // + Add
        Add( in->right, out, error, errormsg, errorNode ) ; // р把计常癬ㄓ 璝Τ岿穦ㄤい砞﹚errormsg
      } // if +
      else if ( in->left->str == "-" ) { // - Sub
        Sub( in->right, out, error, errormsg, errorNode ) ;
      } // else if -
      else if ( in->left->str == "*" ) { // Multi
        Multi( in->right, out, error, errormsg, errorNode ) ;
      } // else if *
      else if ( in->left->str == "/" ) { // Div
        Div( in->right, out, error, errormsg, errorNode ) ;
      } // else if Div
      else if ( in->left->str == "not" ) { // NOT
        in = in->right ;
        if ( in->left->token.type == NIL ) {
          out = t ;
        } // if 
        else {
          out = f ;
        } // else 
      } // else if NOT
      else if ( in->left->str == "and" ) { // And
        And( in->right, out, error, errormsg, errorNode, useLocal ) ;
      } // else if And
      else if ( in->left->str == "or" ) { // Or
        Or( in->right, out, error, errormsg, errorNode, useLocal ) ;
      } // else if Or
      else if ( in->left->str == ">"  ) { // >
        BiggerThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >
      else if ( in->left->str == ">=" ) { // >=
        BiggerEquThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >=
      else if ( in->left->str == "<"   ) { // <
        SmallerThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >
      else if ( in->left->str == "<="  ) { // <=
        SmallerEquThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >=
      else if ( in->left->str == "="  ) { // =
        Equ( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if =
      else if ( in->left->str == "string<?"  ) { // string<?
        StrSmallerThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >
      else if ( in->left->str == "string>?" ) { // string>?
        StrBiggerThan( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if >=
      else if ( in->left->str == "string=?" ) { // string=?
        StrEqu( in->right, out, error, errormsg, t, f, errorNode ) ;
      } // else if =
      else if ( in->left->str == "string-append" ) {
        StrAppend( in->right, out, error, errormsg, errorNode ) ;
      } // else if
 
    } // else if OPERATOR_FN(+-*/)
    else if ( in->left->token.funcType == EQU_TEST ) { // EQU_TEST
      NodePtr t = new Node() ;
      t->str = "#t" ;
      t->token.type = T ; // 獶盽璶 ﹚璶砞﹚type
      t->left = NULL ;
      t->right = NULL ;
      NodePtr f = new Node() ;
      f->str = "nil" ;
      f->token.type = NIL ;
      f->left = NULL ;
      f->right = NULL ;
      
      if ( in->left->str == "eqv?" ) { // eqv?
        Eqv( in->right, out, error, errormsg, t, f ) ;
      } // if eqv?
      else { // equal?
        in = in->right ;
        NodePtr para1 = in->left ; // 材把计
        in = in->right ; 
        NodePtr para2 = in->left ; // 材把计

        out = Equal( para1, para2, t, f ) ;
      } // else equal?
    } // else if  
    else if ( in->left->token.funcType == BEGIN_FN ) { // BEGIN_FN
      in = in->right ;
      out = in ;
      while ( in != NULL && in->token.type != NIL ) {
        out = in->left ;
        in = in->right ;
      } // while 
    } // else if BEGIN_FN
    else if ( in->left->token.funcType == COND_FN ) { // COND_FN (if cond )
      if ( in->left->str == "if" ) {
        If( in, out, error, errormsg, errorNode, useLocal, copy_tree ) ;
      } // if 
      else { // cond
        Cond( in, out, error, errormsg, errorNode, useLocal, copy_tree ) ;
      } // else cond
    } // else if // COND_FN (if cond )
    else if ( in->left->token.funcType == CLEAN_ENVIRONMENT ) { // clean_env
      out = new Node() ;
      out->left = NULL ;
      out->right = NULL ;
      out->str = "environment cleaned" ;
      out->token.funcType = NONE ;
      out->token.type = STRING ;
      gSymbolTAB.clear() ;
      gFunctionTAB.clear() ;
      glocal.clear() ;
      glocalTemp.clear() ; 
    } // else if
    else if ( in->left->token.funcType == LET ) {
      NodePtr bone = in->right ;
      NodePtr temp = NULL ; 
      bool useLocal = true ; // 琌璶ノlocal variables 
      glocalTemp = glocal ;
      DefineSym( bone->left ) ; // ﹚竡local variables
      bone = bone->right ;
      while ( bone != NULL && bone->token.type != NIL ) { // 秨﹍磅︽璶暗ㄆ 
        temp = bone->left ;
        Eval( temp, out, error, errormsg, errorNode, useLocal ) ;
        bone = bone->right ; 
      } // while 

      glocal.clear() ;
    } // else if
    else if ( in->left->token.funcType == LAMBDA ) {
      out = in ;
    } // else if  
    else if ( in->left->token.funcType == EXIT ) {
      out = NULL ;
    } // else if  
    else if ( in->left->token.funcType == VERBOSE ) {
      in = in->right ;
      if ( in->left->token.type == NIL ) {
        gVerbose = false ;
        out = new Node() ;
        out->str = "nil" ;
        out->token.type = NIL ;
        out->left = NULL ;
        out->right = NULL ;
      } // if 
      else {
        gVerbose = true ;
        out = new Node() ;
        out->str = "#t" ;
        out->token.type = T ; // 獶盽璶 ﹚璶砞﹚type
        out->left = NULL ;
        out->right = NULL ;
      } // else 
    } // else if
    else if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
      // (( lambda XXXXX ...
      NodePtr root = in ; 
      NodePtr para2 = in->right ; // 璶砆﹚竡把计 穦砆para2(para2硂ㄇ┮bone) 
      NodePtr para1 = in->left->right->left ; // 把计bone

      glocalTemp = glocal ;
      DefineLambdaLocal( para1, para2, error, errormsg, errorNode ) ;
      if ( error && errormsg == "ERROR (incorrect number of arguments) : " ) {
        errorNode = in->left->left ;
      } // if 

      in = root->left->right->right ; // lambda程把计 碞琌璶磅︽よ 
      while ( in != NULL && error == false ) {
        Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
        in = in->right ;
      } // while 

      glocal = glocalTemp ; // 磅︽Чlambda 碞盢glocal確 
    } // else if
    else if ( in->left->token.funcType == CUSTOMIZE ) {
      DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
      // 肚function把计(癬﹍)┮bone
      NodePtr fnDefine = GetFnDefine( in->left->str ) ; // 眔function璶暗ㄆ 
      useLocal = true ;
      if ( error == false ) { 
        Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // 磅︽function 
      } // if
    } // else if 
  } // Evaluate()

  bool IsExitTree( NodePtr tree ) {
    if ( tree != NULL && tree->left != NULL ) {

      if ( tree->left->str == "exit" ) {
        if ( tree->right == NULL ) return true ;
        else if ( tree->right->token.type == NIL ) return true ;
        else return false ;
      } // if
    } // if

    return false ; 
  } // IsExitTree()
  // ------------------------------------------------------------------------------------- 
  void GetLine() {
    char ch ;
    string str = "" ; // 锣Θstring秈(vector)gLineい 
    bool exit = false ;
    bool error = false ; // ちToken┪琌ゅ猭耞Τ礚岿粇ERROR
    string errormsg = "" ; 
    bool isComplete = true ; // 琌Ч俱攫狥﹁ 
    while ( exit == false && scanf( "%c", &ch ) != EOF  ) { // 璝临⊿弄Ч

      if ( ch == '\n' ) { // 传︽璶秈 耞ERROR琌材碭︽ 
        str = str + ch ; // 锣Θstring 
        gLine.push_back( str ) ;
        str = "" ;
        GetToken( error ) ; // ちtokenだ摸
        gLine.clear() ; 

        // ㄓ 俱︽だ摸Ч
        // ゅ猭耞 璝俱︽临ぃ镑碞单弄︽ 
        error = false ;
        int index = 0 ; // gTokenindex
        isComplete = true ;
        bool isFirstToken = true ;
        bool isEOF = false ;
        bool hasQuote = false ;
        SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

        if ( error ) { // ゅ猭岿粇ERROR 
          cout << errormsg ;
          int k = 0 ;
          while ( ! gToken.empty() && k <= index ) { // 睲岿よ   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

            
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // 睲奔硂︽ 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
            
          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // 睲奔传︽ 

          gLine.clear() ;
          cout << "\n> " ; 
        } // if 
        else { // ゅ猭礚粇 
          if ( IsExitToken() ) exit = true ;
            
          gTree = NULL ;
          NodePtr tree = gTree ;
          int i = 0 ; // index of token
          bool isLeftChild = false ;
          bool isRightChild = false ;
          int lastIndexOfToken = -1 ; // 癘攫gToken
          int lp = 0 ; // オ珹腹计
          int rp = 0 ; // 珹腹计 讽オ珹腹计癸嘿玥ボΘ攫
          string parent = "" ;
          int limitIndex = index ; // 攫硂柑
          bool useLocal = false ; // 琌ㄏノlocal Variables 
          if ( isComplete ) {
            if ( hasQuote ) { // ΤQuote惠璶Quote玡オ珹腹 
              int count = SortOutQuote( limitIndex ) ;
              limitIndex = count + limitIndex ;
            } // if 
            
            BuildTree( tree, i, isLeftChild, isRightChild, lastIndexOfToken, lp, rp, 
                       parent, limitIndex ) ;
            // PrettyPrint( gTree, error ) ;
            tree = NULL ;               
            tree = gTree ;
            NodePtr out = NULL ;
            NodePtr errorNode  = NULL ; // Τ岿粇璶s-exp硂柑 
            // result(pointer)璶狥﹁(EVAL挡狦) 
            Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
            if ( IsExitTree( tree ) ) {
              exit = true ; // 挡 
            } // if 
            else if ( error && errorNode == NULL ) cout << errormsg ;
            else if ( error ) {
              cout << errormsg ;
              if ( errormsg == "ERROR (attempt to apply non-function) : " ) error = false ;
              PrettyPrint( errorNode, error ) ;
            } // else if 
            else {
              PrettyPrint( out, error ) ;
            } // else 
                
            if ( isFirstToken ) limitIndex-- ;  
            DeleteGToken( limitIndex ) ; // 睲奔场だgToken
            delete gTree ;
            gTree = NULL ;
            glocal.clear() ;
            if ( exit == true ) ;
            else cout << "\n> " ;  
          } // if          
        } // else ゅ猭礚粇            

        error = false ; // 穝error (init)
        errormsg = "" ;     
      } // if       
      else {
        str = str + ch ; // 锣Θstring 
        gLine.push_back( str ) ;
        str = "" ;  
      } // else 
    } // while 


    /*
    if ( gLine.size() != 0 && gLine.at( gLine.size() - 1 ) != "\n" ) {

      GetToken( error ) ; // ちtokenだ摸

      // ㄓ 俱︽だ摸Ч
      // ゅ猭耞 璝俱︽临ぃ镑碞单弄︽ 
      if ( error ) {
        gToken.clear() ;
        cout << "\n> " ;
      } // if
    } // if  
    */
    
    GetToken( error ) ; // ちtokenだ摸
    Token gtemp ;
    gtemp.str = "\n" ;
    gtemp.lastIndexOfLine = 0 ;
    gtemp.type = ENTER ;
    gToken.push_back( gtemp ) ;
    gtemp.str = "EOF" ; 
    gtemp.type = EOFILE ;
    gToken.push_back( gtemp ) ;
    bool end = false ;
    error = false ;
    
    while ( gToken.size() > 0 && end == false && exit == false ) { // 
      int index = 0 ; // gTokenindex 
      isComplete = true ;
      error = false ;
      bool isFirstToken = true ;
      bool isEOF = false ;
      bool hasQuote = false ; 
      SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

      if ( error ) { // ゅ猭岿粇ERROR  
        if ( isEOF ) { // 挡 
          gToken.clear() ; 
        } // if 
        else {
          cout << errormsg ;
          int k = 0 ;
        
          while ( k <= index ) { // 睲岿よ   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

          
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // 睲奔硂︽ 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
          

          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // 睲奔传︽ 

          cout << "\n> " ;
          errormsg = "" ;
        } // else 
      } // if ゅ猭岿粇ERROR 
      else { // ゅ猭礚粇
        if ( IsExitToken() ) end = true ;
        
        if ( end == true ) gToken.erase( gToken.end() ) ;
        gTree = NULL ;
        NodePtr tree = gTree ; 
        int i = 0 ; // index of token
        bool isLeftChild = false ;
        bool isRightChild = false ;
        int lastIndexOfToken = -1 ; // 癘攫gToken
        int lp = 0 ; // オ珹腹计
        int rp = 0 ; // 珹腹计 讽オ珹腹计癸嘿玥ボΘ攫
        string parent = "" ;
        int limitIndex = index ; // 攫硂柑
        bool useLocal = false ;
        if ( isComplete ) { // isComplete
          if ( hasQuote ) {
            int count = SortOutQuote( limitIndex ) ;
            limitIndex = count + limitIndex ;
          } // if 
          
          BuildTree( tree, i, isLeftChild, isRightChild, lastIndexOfToken, lp, rp, 
                     parent, limitIndex ) ;

          tree = gTree ;
          // if ( IsExitTree( tree ) ) end = true ;
          NodePtr out = NULL ;
          NodePtr errorNode  = NULL ; // Τ岿粇璶s-exp硂柑 
          // result(pointer)璶狥﹁(EVAL挡狦) 
          Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
          if ( IsExitTree( tree ) ) {
            exit = true ; // 挡 
          } // if 
          else if ( error && errorNode == NULL ) cout << errormsg ;
          else if ( error ) {
            cout << errormsg ;
            // non-function 璶#<Procedure> 硂︽ ┮砞﹚Θno error Τ翴ぃ絋﹚! 
            if ( errormsg == "ERROR (attempt to apply non-function) : " ) error = false ;
            PrettyPrint( errorNode, error ) ;  
          } // else if 
          else {
            PrettyPrint( out, error ) ;
          } // else 

          if ( isFirstToken ) limitIndex-- ; 
          DeleteGToken( limitIndex ) ; // 睲奔场だgToken
          delete gTree ;
          gTree = NULL ;
          glocal.clear() ;
          if ( end == true ) ;
          else cout << "\n> " ; 
        } // if // isComplete          
      } // else 
    } // while 

    if ( end == false && exit == false ) { // 
      cout << "ERROR (no more input) : END-OF-FILE encountered" ;
    } // if 
  } // GetLine()


};
void Project3() {
  char ch ;
  scanf( "%d%c", &gTestNum, &ch ) ;
  OurScheme o ;
  o.GetLine() ;
} // Project3()

// --------------------------------------------------------------------------------------------------
int main() {
  cout << "Welcome to OurScheme!\n\n> " ;
  Project3() ; 
  cout << "\nThanks for using OurScheme!" ;  
  return 0 ;
} // main()
