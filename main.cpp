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
  string str ; // 他本人
  bool error ; // 是否錯誤 
  string errormsg ; // 錯誤訊息 
  TerminalToken type ; // 分類
  FunctionType funcType ; // 函式分類 
  int lastIndexOfLine ; // 在(vector)gLine中的index(存最後一個char所在位置) 
}; 

struct Node{
  Token token ;
  string str ; // 他本人
  Node * left ; // 左pointer 
  Node * right ; // 右pointer
  int ans_int ;
  float ans_float ; // 是float就直接拿float運算 
};

typedef Node * NodePtr ;

struct Symbol{
  string str ; // symbol的名稱
  Node * value ; // Symbol要被定義的值 
};
struct Fn{
  string str ; // Function名稱
  int numOfArgs ; // 有多少參數
  string originFn ; // 這是被哪個functionn所定義的 
  vector<string> args ; // 參數(名稱)存在這個裡面 
  Node * value ; // 該fn的定義(要做的事)
};



int gTestNum = 0 ;
bool gVerbose = true ; // 特殊要實作的...跟define clean evn.要不要印有關 
vector<string> gLine ; // 存讀入的東西(基本上是一行) 
vector<Token> gToken ; // 要切Token的Token串
vector<Symbol> gSymbolTAB ; // symbol Table 定義過的symbol都會存在這裡 
vector<Fn> gFunctionTAB ; // 定義過的function都存在這裡 提供查詢用 
vector<Symbol> glocal ; // let會用到的local variable都匯存在這裡 


NodePtr gTree ; // 樹樹本樹 

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

  bool IsEscape( int index ) { // ' " \ 這三個case需要escape (\n, \t留著即可)   
    if ( gLine.at( index ) == "'"  || gLine.at( index ) == "\"" || gLine.at( index ) == "\\"
         || gLine.at( index ) == "n"  || gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscape()

  bool IsEscapeN( int index ) { // ' " \ 這三個case需要escape (\n, \t留著即可)   
    if ( gLine.at( index ) == "n" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeN()

  bool IsEscapeT( int index ) { // ' " \ 這三個case需要escape (\n, \t留著即可)   
    if ( gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeT()

  bool IsNumber( string temp ) { // 參考https://www.itread01.com/content/1546245258.html 
    char ch ;
    stringstream ss( temp ) ;
    double isDouble ;
    
    if ( ! ( ss >> isDouble ) ) { // ss >> isDouble把 ss轉換成double的變數
      return false ; // (int/ float都接)，傳換失敗則值為0 
    } // if 
    
    if ( ss >> ch ) { // 檢測錯誤輸入(數字加字串)(例如：12.a）
      return false ; // 此時接收.a的部分 (條件成立，所以return false) 
    } // if 

    return true ;
  } // IsNumber()
  
  int CountEnter( int index ) { // 算目前尚未處理的東西中有幾個換行 
    int counter = 0 ;
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gToken.size() && toBreak == false ) {
      if ( gToken.at( i ).str == "\n" ) {
        counter++ ;
      } // if

      if ( i == index ) { // 遇到本人 加 1(此行)並結束 
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
      if ( temp == "\n" ) { // 換行 
        gtemp.type = ENTER ;
      } // if 
      else if ( temp == "(" ) { // 左括號 
        gtemp.type = LEFT_PAREN ;
        int j = indexOfLine + 1 ;
        while ( j < gLine.size() && ( gLine.at( j ) == " " || gLine.at( j ) == "\t" ) ) j++ ; 
        if ( j < gLine.size() && gLine.at( j ) == ")" ) { // () nil的狀況 
          gtemp.type = NIL ;
          gtemp.str = "nil" ;
          indexOfLine = j ;
          gtemp.lastIndexOfLine = j ;
        } // if 
      } // else if (左括號) 
      else if ( temp == ")" ) { // 右括號 
        gtemp.type = RIGHT_PAREN ;
      } // else if (右括號)
      else if ( temp == "'" ) { // (單)引號 Quote 
        gtemp.type = QUOTE ;
        gtemp.funcType = QUOTE_FN ;
      } // else if (單)引號 Quote
      else if ( temp == "nil" || temp == "#f" ) { // NIL
        gtemp.type = NIL ;
      } // else if (nil)
      else if ( temp == "t" || temp == "#t"  ) { // T
        gtemp.str = "#t" ;
        gtemp.type = T ;
      } // else if (T)
      else if ( temp == ";" ) { // 註解
        gtemp.type = COMMENT ;  
        while ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) != "\n" ) { // 讀到換行 
          indexOfLine++ ;
        } // while

        if ( indexOfLine < gLine.size() ) 
          indexOfLine-- ; // 回去後會++, 再次回到換行    
      } // else if (註解)
      else if ( temp == "\"" ) { // String 
        // string lastStr = "" ; // 存上一個遇到的TOKEN
        bool toBreak = false ; 
        gtemp.str = temp ; // "(雙)引號            
        indexOfLine++ ;
        while ( indexOfLine < gLine.size() && toBreak == false ) { // 直到讀到第二個(雙)引號 
          if ( gLine.at( indexOfLine ) == "\"" ) { // 字串結束的右引號 
            toBreak = true ;
          } // if 
          else if ( gLine.at( indexOfLine ) == "\\" ) { // 反斜線 
            if ( indexOfLine + 1 >= gLine.size() ) toBreak = true ;
            else if ( IsEscape( indexOfLine + 1 ) ) { // 判斷此反斜線是否為escape意義 
              indexOfLine++ ; // 跳過有escape定義的反斜線  如: 目前為 \" 的 "  
            } // else if 

            char enter = '\n' ; // 換行
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
          else { // string的部分 
            gtemp.str = gtemp.str + gLine.at( indexOfLine ) ;
            indexOfLine++ ;
          } // else            
        } // while 
      
        if ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) == "\"" ) { // STRING格式正確       
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
      else if ( IsNumber( temp ) ) { // 數字(int/float) 

        if ( temp[0] == '+' ) {
          temp.erase( 0, 1 ) ; // 從index = 0開始，刪除1個char
        } // if 


        gtemp.str = "" ;        
        bool isFloat = false ;
        int i = 0 ;
        int indexOfDot = 0 ;
        char lastCH = '\0' ;
        while ( i < temp.length() ) { // 判斷是int還是float
 
          if ( temp[i] == '.' ) {
            indexOfDot = i ;
            isFloat = true ;
            if ( lastCH == '-' ) {
              gtemp.str = gtemp.str + "0" ;
            } // else if

            if ( i == 0 ) { // 若是.1 => 0.1 前面補零 
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
          int num = temp.length() - indexOfDot - 1 ; // 後面有幾位數
          int n = 3 - num ; // 要補給個0
          while ( n > 0 ) {
            gtemp.str = gtemp.str + "0" ;
            n-- ;
          } // while
    
        } // if  
      } // else if (數字(int/float))
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

      if ( error ) { // 若string錯 則最後面也要放ENTER 
        gtemp.str = "\n" ; 
        gtemp.type = ENTER ;
        gToken.push_back( gtemp ) ;
      } // if 
    } // else 
  } // Classify()
    
  void GetToken( bool &error ) { // 分類 => Token串  
    bool toBreak = false ;
    int indexOfLine = 0 ; // index of (vector) gLine(string)
    int indexOfToken = 0 ; // index of (vextor) gToken(Token)
    int countEnter = 0 ; // 算有幾個換行 
    string temp = "" ;
    while ( toBreak == false && indexOfLine < gLine.size() ) { // 還沒分類完 

      if ( gLine.at( indexOfLine ) == "\n" ) { // 換行
        if ( temp != "" ) { // 前面有Token要處理 
          indexOfToken++ ; // 換下一個TOKEN 
          indexOfLine-- ; // 跳回Token的地方 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // 回到換行這裡 
          temp = "" ;
        } // if  


 
        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // 換下一個TOKEN 
        Classify( temp, indexOfLine, indexOfToken, error ) ;
        temp = "" ; 
 
        toBreak = true ;
      } // if (換行) 
      else if ( IsSeparator( gLine.at( indexOfLine ) ) ) { // Separator
        if ( temp != "" ) { // 前面有Token要處理 
          indexOfToken++ ; // 換下一個TOKEN
          indexOfLine-- ; // 跳回Token的地方 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // 回到換行這裡
          temp = "" ;
        } // if 

        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // 換下一個TOKEN 
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
      indexOfToken++ ; // 換下一個TOKEN 
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
                       bool & isFirstToken, bool & isEOF, bool & hasQuote ) { // 文法分析 


    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // 換行
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 


      index++ ;

    } // while 換行

    if ( index < gToken.size() && gToken.at( index ).error == true ) { // 若是string錯誤 
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

    if ( error == true || index >= gToken.size() ) { // 結束!或是說不做事
      if ( index >= gToken.size() ) isComplete = false ;

      return true ; 
    } // if

    
    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // 換行或註解 
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 

      index++ ;

    } // while 換行

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
        // 文法錯直接結束
        error = true ;
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( index < gToken.size() && gToken.at( index ).type == LEFT_PAREN ) { 
      // 左括號 <S-exp>{<S-exp>}[DOT<S-exp>]右括號
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
      if ( j < gToken.size() && gToken.at( j ).type == RIGHT_PAREN ) { // ()的狀況 
        gToken.at( index - 1 ).str = "nil" ;
        gToken.at( index - 1 ).type = NIL ;
        gToken.at( index - 1 ).lastIndexOfLine = j ;
        int k = index ;
        while ( k <= j ) { // 消滅 
          gToken.erase( gToken.begin() + index ) ;
          k++ ;
        } // while 

        index = index - 1 ;
        isNIL = true ;
        return true ;
      } // if ()的狀況 
      else if ( ! SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ) 
        return false ;

      if ( isNIL == false )
        index++ ; // 判斷下一個 
      while ( index < gToken.size() && gToken.at( index ).type != DOT 
              && gToken.at( index ).type != RIGHT_PAREN ) { // 為<S-exp> 去判斷<S-exp>是否正確
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
      else if ( index < gToken.size() && gToken.at( index ).type == DOT ) { // DOT後面只能接 ATOM/ 左括號 / 單引號 
        index++ ;
        if ( index >= gToken.size() ) {
          isComplete = false ;
          return true ;
        } // if 

        if ( index < gToken.size() && IsATOM( gToken.at( index ).type ) == false && 
             gToken.at( index ).type != LEFT_PAREN 
             &&  gToken.at( index ).type != QUOTE && gToken.at( index ).type != ENTER && 
             gToken.at( index ).type != COMMENT ) {
        // 若DOT後不是ATOM / 左括號 / 單引號 / 換行 / 註解 
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
          else index++ ; // 往下判斷

          if ( index < gToken.size() && gToken.at( index ).type == ENTER ) index++ ; 
        } // if 
        //  DOT後面接 ATOM/ 左括號 / 單引號 則去判斷該行文法 
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
        else if ( index < gToken.size() && gToken.at( index ).type != RIGHT_PAREN ) { // DOT<S-exp>後沒有右括號
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
      } // else if // DOT後面只能接 ATOM/ 左括號 / 單引號
      else if ( index < gToken.size() && gToken.at( index ).type == RIGHT_PAREN ) { // 右括號 ->結束
        isComplete = true ; 
        return true ;
      } // else if 右括號 ->結束
      else if ( isNIL == false ) return false ;

    } // else if 左括號
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
                  int & LP, int & RP, string parent, int limitIndex ) { // 建樹 當左右括號數對稱，則表示成功建樹 

    if ( i >= gToken.size() || i > limitIndex ) { // 結束
      ;
    } // if 結束
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
    else if ( i < gToken.size() && gToken.at( i ).type == RIGHT_PAREN ) { // 遇到右括號
      RP++ ; // 右括號數 + 1
      lastIndexOfToken = i ;
      if ( isLeftChild ) {
        i++ ;
        BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // if
      else {
        // i++ ;
        // BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // else  
    } // else if 遇到右括號
    else {
      if ( IsATOM( gToken.at( i ).type ) ) { // 遇到ATOM
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->token = gToken.at( i ) ;
          tree->str = gToken.at( i ).str ;
          tree->left = NULL ;
          tree->right = NULL ; 
        } // if  
        else if ( isLeftChild ) { // 是左小孩
          parent = tree->str ; 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;
        } // if 左小孩
        else if ( isRightChild ) { // 是右小孩
          parent = tree->str ;
          if ( parent == "." && gToken.at( i ).type == NIL ) { // 若為 ".nil" 則不放點(這裡覆蓋即可
            tree->str = "MimiNote:DontNeedToPutTwoSpace" ;     
          } // if 

          tree->right = new Node() ;
          tree = tree->right ;
          tree->left = NULL ;
          tree->right = NULL ;
          if ( parent == "." ) { // 有DOT就直接放
            tree->str = gToken.at( i ).str ;
            tree->token = gToken.at( i ) ;
          } // if 
          else { // 沒DOT ->多放一NODE
            tree->str = "" ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 往左多放一node
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 出來再往右 
          } // else         
        } // else if 右小孩

        lastIndexOfToken = i ;
      } // else if  遇到ATOM
      else if ( gToken.at( i ).type == DOT ) { // 遇到點
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
          i++ ; // 只往右走
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
          lastIndexOfToken = i ;
          int index = i + 1 ;
          bool toBreak = false ;
          while ( gToken.at( index ).str == "\n" ) { // 換行都跳過 
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
      
      } // else if 遇到點
      else if ( gToken.at( i ).type == QUOTE ) { // 遇到QUOTE

        if ( isLeftChild ) { // 只會是左小孩 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = "quote" ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;

        } // if 

      } // else if 遇到QUOTE
      else if ( gToken.at( i ).type == LEFT_PAREN ) { // 遇到左括號
        // bool rightChildDone = true ;
        LP++ ; // 算左括號數量
        bool done = false ;
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ; 

          // 只要是左括號 後面都這樣做(放好之後 先往左走 再往右)
          if ( done == false ) {
            tree->str = "(" ;
            tree->left = NULL ;
            tree->right = NULL ;
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 先往左走
            lastIndexOfToken = i ;
          } // if

          i++ ;
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 再往右走
          lastIndexOfToken = i ;
        } // if 
        else {
 
          parent = tree->str ;
          if ( parent == "." ) { // 一定是右小孩 但是不需要多一個NODE
            tree->str = "MimiNote:NeedToPutTwoSpace" ; // 若為 ".(" 則不放點(這裡覆蓋即可
            done = true ; 
            // 只要是左括號 後面都這樣做(放好之後 先往左走 再往右)
            if ( done == false ) {
              tree->str = "(" ;
              tree->left = NULL ;
              tree->right = NULL ;
              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 先往左走
              lastIndexOfToken = i ;
            } // if

            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 再往右走
            lastIndexOfToken = i ;


          } // if 
          else { // parent不是點
            if ( isRightChild ) { // 右小孩要多做一步(多建一node)
              parent = tree->str ;
              tree->right = new Node() ;
              tree = tree->right ;
              tree->str = "" ;
              tree->left = NULL ;
              tree->right = NULL ;


              // 只要是左括號 後面都這樣做(放好之後 先往左走 再往右)
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                // i++ ;
                parent = tree->str ; 
                // if ( i + 1 < gToken.size() && gToken.at( i+1 ).type == QUOTE ) i++ ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 先往左走
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 再往右走
              lastIndexOfToken = i ;


            } // if 右小孩
            else { // 左小孩 

              parent = tree->str ;
              tree->left = new Node() ; // 左右小孩都要往左建(if parent不是點)
              tree->right = NULL ;
              tree = tree->left ;
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                i++ ;
                parent = tree->str ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 先往左走
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // 再往右走
              lastIndexOfToken = i ;
            } // else 左小孩 
          } // else parent不是點         
        } // else 
      

      } // else if 遇到左括號
    } // else 
  } // BuildTree()

  int SortOutQuote( int limitIndex ) { // 若有quote 就將quote的前後都加括號 'a b = ('a ) b 
    int i = 0 ; // index of gToken
    stack<int> indexToInsertLP ; // 插LP的地方 
    Token gtempRP ; // RP
    gtempRP.str = ")" ; 
    gtempRP.error = false ;
    gtempRP.type = RIGHT_PAREN ;
    Token gtempLP ; // LP
    gtempLP.str = "(" ;
    gtempLP.error = false ;
    gtempLP.type = LEFT_PAREN ; 
    int count = 0 ; // 計算加了多少東西
 
    while ( i < gToken.size() && i <= limitIndex ) {
      if ( gToken.at( i ).type == QUOTE && gToken.at( i ).str == "'" ) { // quote的地方就是插LP的地方 
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
        else if ( IsATOM( gToken.at( i ).type ) ) { // quote後是ATOM 
          int j = i + 1 ;
          gtempRP.lastIndexOfLine = gToken.at( i ).lastIndexOfLine ;
          if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
          else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RP在ATOM後 
          count++ ; 
        } // else if quote後是ATOM
        else if ( gToken.at( i ).type == LEFT_PAREN ) { // quote後面是左括號  
          int j = i + 1 ;
          int lp = 1 ; // 左括號數量 
          int rp = 0 ; // 右括號數量 
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
              gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP在(已對稱的)右括號後 
              count++ ;
              toBreak = true ;
            } // if 

            j++ ; 
          } // while 
        } // else if quote後面是左括號 
        else if ( gToken.at( i ).type == QUOTE ) { // 連續的QUOTE 
          int k = i ;
          while ( k < gToken.size() && gToken.at( k ).type == QUOTE ) { // 跑到不是QUOTE即可 
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
            if ( IsATOM( gToken.at( k ).type ) ) { // quote後是ATOM 
              int j = k + 1 ;
              gtempRP.lastIndexOfLine = gToken.at( j ).lastIndexOfLine ;
              if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
              else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RP在ATOM後
              count++ ; 
            } // if quote後是ATOM
            else if ( gToken.at( k ).type == LEFT_PAREN ) { // quote後面是左括號  
              int j = k + 1 ;
              int lp = 1 ; // 左括號數量 
              int rp = 0 ; // 右括號數量 
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
                  gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP在(已對稱的)右括號後
                  count++ ; 
                  toBreak = true ;
                } // if 
 
                j++ ; 
              } // while 
            } // else if quote後面是左括號 
          } // else 
        } // else if 連續的QUOTE 

        i-- ; // 回到下一個QUOTE(等下會i++) 
      } // if quote的地方就是插LP的地方 (有遇到quote)

      i++ ; 
    } // while

    while ( ! indexToInsertLP.empty() ) { // 開始insert LP(在quote的index ) 
      int index = indexToInsertLP.top() ;
      gtempLP.lastIndexOfLine = gToken.at( index ).lastIndexOfLine ;
      gToken.insert( gToken.begin()+index, gtempLP ) ;
      count++ ;
      indexToInsertLP.pop() ;
    } // while 

    return count ;
  } // SortOutQuote()


  void ReCalc( int index, int j ) { // // 從第index個開始，每個lastIndexOfLine皆減去 (j+1)
    while ( index < gToken.size() && gToken.at( index ).str != "\n" ) {
      gToken.at( index ).lastIndexOfLine = gToken.at( index ).lastIndexOfLine - j - 1 ;
      index++ ;
    } // while 
  } // ReCalc()

  void DeleteGToken( int lastIndexOfToken ) { // 刪掉已建好樹的gToken
    int i = 0 ;
    int lastIndexOfLine = 0 ; 
    if ( lastIndexOfToken == -1 ) ;
    else if ( lastIndexOfToken >= gToken.size() ) { // 全清空
      gToken.clear() ; 
    } // else if 
    else { // 部分清空
      lastIndexOfLine = gToken.at( lastIndexOfToken ).lastIndexOfLine ;
      while ( ! gToken.empty() && i <= lastIndexOfToken ) {

        gToken.erase( gToken.begin() ) ; // 刪掉第一個之後，後面會遞補上來，所以永遠刪第一個
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
              bool error ) { // 每印個東西 就印空格 除左括號外 其他印換行

    if ( tree == NULL ) { // 右邊是NULL (左邊不可能是NULL) 
      rp++ ;
      int times = lp - rp ;
      while ( times > 0 ) {
        cout << "  " ; // 印兩個空格
        times-- ;
      } // while

      if ( parent != "OMGISCOMMENT" ) {
        cout << ")\n" ;
        parent = ")" ;
      } // if 
    } // if 
    else if ( tree->left == NULL && tree->right == NULL ) { // 走到底了
      int times = 0 ;
      bool hasPrintSpace = false ;   
      if ( isRightChild && tree->token.type == NIL ) { // 走到底且本人是NULL
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
      else { // 走到底 本人是其他東西
        if ( parent == "(" ) { // 若上一個是左括號 就不用印空白
          ;
        } // if
        else { // 上一個不是左括號 所以要印空白      
          times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // 印兩個空格
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

      if ( isRightChild ) { // 右邊走到底 代表有遇到右括號
        int times = lp - 1 ;
        while ( times > 0 ) {  
          cout << "  " ;
          times-- ;
        } // while 

        cout << ")\n" ;
        parent = ")" ;
        rp++ ;
      } // if 
    } // else if 走到底了
    else { 
      bool dontNeedSpace = false ; 
      if ( isLeftChild ) { // 尚未走到底 且為左邊Node
        // if ( parent != "(" && parent != "" ) {
        if ( parent != "(" ) {
          int times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // 印兩個空格
            times-- ;
          } // while
        } // if 

        cout << "( " ;
        lp++ ;
        parent = "(" ; 
      } // if 


      Print( tree->left, true, false, lp, rp, parent, error ) ; // 左

      if ( ( tree->str != "." && tree->str != "" && tree->str != "(" 
           )
           || tree->right == NULL
           || tree->str == "MimiNote:DontNeedToPutTwoSpace"
           || tree->right->token.type == NIL ) 
        dontNeedSpace = true ;
                           
      if (  dontNeedSpace || parent == "(" || parent == "" || tree->str == "" ) { 
      // 若上一個是左括號 就不用印空白  
        ;
      } // if
      else if ( tree->str == "MimiNote:NeedToPutTwoSpace" ) {
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // 印兩個空格
          times-- ;
        } // while
      } // else if 
      else { // 上一個不是左括號 所以要印空白      
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // 印兩個空格
          times-- ;
        } // while
        
        
      } // else   

      if ( tree->str != "(" && tree->str != "" && tree->str != "MimiNote:DontNeedToPutTwoSpace" 
           && tree->str != "MimiNote:NeedToPutTwoSpace" ) // 根
        cout << tree->str << "\n" ;

      parent = tree->str ;
      Print( tree->right, false, true, lp, rp, parent, error ) ; // 右
    } // else
  } // Print()
  // ------------------------------------------------------------------------------------------- 
  
  bool IsSExp( NodePtr cur ) { // 判斷cur指向的子樹是否為S-Expression 
    if ( cur == NULL ) return true ; 
    else if ( cur != NULL && IsATOM( cur->token.type ) ) { // Atom
      return true ; 
    } // else if ATOM
    else if ( cur != NULL && cur->token.type == QUOTE ) { // QUOTE <S-exp>
      if ( IsSExp( cur->right ) == false ) { // 讓index跑到該結束的地方
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( cur != NULL && ( cur->token.type == LEFT_PAREN || cur->token.type == DOT ) ) { 
      // 左括號 <S-exp>{<S-exp>}[DOT<S-exp>]右括號
      if ( IsSExp( cur->left ) == false ) { // 判斷左子樹 
        return false ;
      } // if 

      if ( IsSExp( cur->right ) == false ) { // 判斷右子樹 
        return false ;
      } // if

      return true ;
    } // else if 左括號


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

  // 判斷是否為定義過的Symbol
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

  // 若為List則右邊走到底是NULL 
  bool IsList( NodePtr tree ) {
    NodePtr pre = tree ;
    while ( tree != NULL ) {   
      pre = tree ;  
      tree = tree->right ;
    } // while 

    if ( pre->token.type == NIL ) return true ;
    else if ( IsATOM( pre->token.type ) ) return false ; // 只能是龍骨，不能是ATOM    
    else return true ;
  } // IsList() 

  // 判斷是否為定義過的Function
  bool IsBoundFunction( Token token ) {  
    // 老大規定的function 
    if ( token.funcType == CONS || token.funcType == QUOTE_FN || token.funcType == DEFINE ||
         token.funcType == PART_ACCESSOR || token.funcType == PRIMITIVE_PREDICATE || 
         token.funcType == BEGIN_FN || token.funcType == OPERATOR_FN || token.funcType == EQU_TEST || 
         token.funcType == COND_FN || token.funcType == CLEAN_ENVIRONMENT || token.funcType == EXIT ||
         token.funcType == LET || token.funcType == LAMBDA ) {
      return true ;
    } // if
    else if ( token.str == "exit" ) { // 不確定對不對 先這樣寫 
      return true ;
    } // else if 
    
    // 其他自定義的Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( token.str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;     
  } // IsBoundFunction()

  bool IsTopLevel( NodePtr cur ) { // 判斷是不是top Level 
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

  bool IsPair( NodePtr cur ) { // ( ( x 5 ) ( y '(1 2 3)) ) LET的判斷 
    while ( cur != NULL && cur->token.type != NIL ) {
      if ( cur->left != NULL && ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) ) {
        return false ;
      } // if 
      else { // ( x 5 ) 判斷括號裡面 
        NodePtr temp = cur->left ; // temp是左括號  
        if ( temp->left != NULL && temp->left->token.type != SYMBOL ) { // 應該要是Symbol 
          return false ;
        } // if  

        temp = temp->right ;
        if ( temp == NULL ) return false ; // 沒有定義值 則 ERROR 
        if ( temp->left != NULL && IsSExp( temp->left ) == false ) { 
          // 判斷下一個地方是不是sexp (應為SExp)
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
              if ( cur != NULL ) { // (define a 10 20 ) 只能有一個參數 ex: ( define a 10 ) 
                if ( cur->token.type == NIL ) {
                  return true ;
                } // if 

                errormsg = "ERROR (DEFINE format) : " ;
                return false ;
              } // if 
            
              return true ; // 後面進evaluation() 
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
          while ( cur != NULL && cur->token.type != NIL ) { // 判斷( F x y ) 是不是都是symbol 
            if ( IsATOM( cur->left->token.type ) == false ) { 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 
            else if ( cur->left->token.type != SYMBOL ) { // 非symbol ( F 3 ) 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // else if

            if ( first && IsProcedureFn( cur->left->token ) ) {  // 不可以定義老大定義的function 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 

            cur = cur->right ;
          } // while 

          bone = bone->right ;
          
          while ( cur != NULL && cur->token.type != NIL ) { // 1~多個 S-Exp 
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
        cur = cur->right ; // // 換下一個龍骨(參數)檢查其左子樹是否是<S-exp>     
      } // while 
      
      if ( ( cur == NULL || cur->token.type == NIL ) && isSExp && isDoubleTon ) { // 文法正確 
        return true ;
      } // if 
      else { // 文法錯誤 
        errormsg = "ERROR (COND format) : " ;
        return false ;
      } // else 
    } // else if
    else if ( type == LET ) { // LET
      int numOfArgs = CountNumOfArgs( cur ) ; // cur 從第一個參數的龍骨開始看 
      if ( numOfArgs != 2 && numOfArgs < 2 ) { // 參數部分只有兩個龍骨 (不算let的部分) 
        errormsg = "ERROR (LET format) : " ;
        return false ;
      } // if 
      else {
        if ( IsATOM( cur->left->token.type ) && cur->left->token.type == NIL ) ;
        else if ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) { 
          // 應該要是一個node 不能是個ATOM (應該要是左括號) 
          errormsg = "ERROR (LET format) : " ;
          return false ;
        } // else if
        else { // 第一個參數是左括號開頭 (zero-or-more-PAIRs)  ( ( x 5 ) ( y '(1 2 3))) 
          if ( IsPair( cur->left ) == false ) { // 傳這個進去判斷 ( ( x 5 ) ( y '(1 2 3)) )
            errormsg = "ERROR (LET format) : " ;
            return false ;
          } // if 
        } // else 
      } // else 
    } // else if LET 
    

    return true ;
  } // CheckFormat()

  int CountNumOfArgs( NodePtr cur ) { // 算argument的數量 
    if ( cur != NULL && cur->left != NULL ) {
      return CountNumOfArgs( cur->right ) + 1 ;
    } // if
    else {
      return 0 ;
    } // else 
  } // CountNumOfArgs()

  void CheckNumOfArgs( FunctionType type, NodePtr in, int numOfArgs, bool &error, string & errormsg ) {
    // lambda的errormsg可能要另外處理 (在此FN中處理) 
    if ( type == CONS ) {
      if ( in->left->str == "cons" && numOfArgs != 2 ) { // cons(2)
        error = true ;  
      } // if 參數數量錯誤
      else if ( in->left->str == "list" && numOfArgs < 0 ) { // list(>=0) 
        error = true ;  
      } // else if 
      else {
        error = false ;
      } // else 
    } // if
    else if ( type == QUOTE_FN ) { // quote(1)
      if ( numOfArgs != 1 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == PART_ACCESSOR ) { // car(1), cdr(1)
      if ( numOfArgs != 1 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if 
    else if ( type == PRIMITIVE_PREDICATE ) { // 都只有1個 如: atom? pair? null? integer?... 
      if ( numOfArgs != 1 ) { // 參數數量錯誤
        error = true ;   
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == OPERATOR_FN ) { // +-*/ > = < not and or...
      if ( in->left->str == "not" && numOfArgs != 1 ) { // not(1)
        error = true ;           
      } // if 參數數量錯誤
      else if ( in->left->str != "not" && numOfArgs < 2 ) { // 其他都要 >= 2 
        error = true ;
      } // else if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EQU_TEST ) { //   eqv?(2) equal?(2)
      if ( numOfArgs != 2 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == BEGIN_FN ) { // begin
      if ( numOfArgs < 1 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "cond" && numOfArgs < 1 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "if" && ( numOfArgs != 2 || numOfArgs != 3 ) ) { // 參數數量錯誤
        error = true ;   
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CLEAN_ENVIRONMENT ) { // clear_env.
      if ( numOfArgs != 0 ) { // 參數數量錯誤
        error = true ;  
      } // if 參數數量錯誤
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CUSTOMIZE ) { // 自定義的FN 
      Fn f ;
      int i = 0 ;
      bool found = false ;
      while ( i < gFunctionTAB.size() && !found ) { // 從gFunctionTAB中找此FN 
        if ( in->left->str == gFunctionTAB.at( i ).str ) { // 找到此FN 
          f = gFunctionTAB.at( i ) ;
          found = true ;
        } // if
        
        i++ ; 
      } // while
      
      if ( numOfArgs != f.numOfArgs ) { // 判斷參數數量對不對 
        error = true ;        
      } // if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EXIT ) {
      if ( numOfArgs != 0 ) { // 判斷參數數量對不對 
        error = true ;        
      } // if
      else error = false ; 
    } // else if 

    if ( error ) {
      errormsg = "ERROR (incorrect number of arguments) : " + in->left->str + "\n" ; 
    } // if error
  } // CheckNumOfArgs()

  bool DoNotNeedEvalARGS( NodePtr in ) { // DEFINE/ QUOTE/ COND/ if/ and/ or 不需要判斷下一個args 所以要跳過

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
    if ( useLocal ) { // 此symbol可能是local  
      while ( i < glocal.size() ) {
        if ( glocal.at( i ).str == node->token.str ) {
          return glocal.at( i ).value ;
        } // if 
      
        i++ ;
      } // while 
    } // if 

    // 若local沒找到 
    i = 0 ;
    while ( i < gSymbolTAB.size() ) {
      if ( gSymbolTAB.at( i ).str == node->token.str ) {
        return gSymbolTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 

    // SYMBOL 沒找到 改找function 
    i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( gFunctionTAB.at( i ).str == node->token.str ) {
        return gFunctionTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 
        
    return node ;
  } // GetDefineSymbolNode()   
  
  bool IsCustomize( string str ) { // 是自定義的function 
    // 其他自定義的Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;
  } // IsCustomize() 

  bool CheckCustomizeArgs( string str, NodePtr in ) { // 判斷自定義的fn的參數是否相同 
     // 其他自定義的Function 
    int i = 0 ;
    int numOfArgs = 0 ; 
    int index = 0 ; // 該自定義的function在哪個位置 
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) {
      if ( str == gFunctionTAB.at( i ).str ) { // 找到了 
        toBreak = true ;
        index = i ;
      } // if 
      
      i++ ;
    } // while

    i = 0 ;
    while ( in != NULL && in->token.type != NIL ) { // 計算(建樹後)有多少參數 
      numOfArgs++ ;
      in = in->right ;
    } // while 

    if ( numOfArgs == gFunctionTAB.at( index ).numOfArgs ) return true ;
    else return false ;  
  } // CheckCustomizeArgs()

  void DefineLocal( string funcName, NodePtr in, bool useLocal, bool & error, string & errormsg, 
                    NodePtr & errorNode ) { 
    // 定義local variable 則function才能做事 
    // (傳function名，參數(起始)所在的bone) 
    int i = 0 ;
    bool toBreak = false ;
    bool find = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // 先找到function，找到要定義的參數們 
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
    NodePtr in_temp = in ; // 紀錄in一開始的位置 
    // 將所有要被定義的值先計算完畢 
    while ( in != NULL && in->token.type != NIL && error == false ) { 
      Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        errormsg = "ERROR (unbound symbol) : " ;
        errorNode = in->left ;
      } // if 

      in->left = out ; // 參數的值 
      in = in->right ; // 換下一個參數 
    } // while 

    // 一一將要被定義的參數取出並，給予值 
    i = 0 ;
    in = in_temp ; 
    while ( error == false && i < f.args.size() ) { // 將參數一一取出，並定義 
      s.str = f.args.at( i ) ; // 參數名稱
      s.value = in->left ; // 參數的值
      if ( s.value != NULL && IsBoundSymbol( s.value->token ) ) {
        DeleteDefineSym( s.str ) ;
      } // if 

      glocal.push_back( s ) ; // 存入glocal中 
      in = in->right ; // 換下一個參數 
      i++ ;
    } // while 

  } // DefineLocal()

  NodePtr GetFnDefine( string fnName ) { // 取得fnName這個function要執行的事 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // 先找到function，找到要定義的參數們 
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

  bool IsProcedureFn_Str( string str, FunctionType & fnType ) { // 若此string是老大定義的finction 回傳type 
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

  bool CheckLambdaFormat( NodePtr in ) { // 檢查lambda的format是否符合 
    // (lambda (zero-or-more-symbols但不是constant) one-or-more-S-expressions )

    NodePtr bone = in->right ; // 參數所在bone 
    if ( bone == NULL || bone->token.type == NIL ) return false ; // ( lambda ) 
    in = in->right ;
    // 先檢查 (zero-or-more-symbols但不是constant)
    if ( IsATOM( in->left->token.type ) && in->left->token.type == NIL ) ; // () 零個symbol
    else if ( IsATOM( in->left->token.type ) ) return false ; // ( lambda 5 )
    else { // 第一個參數 1~多個symbol()，但不是數字那類的只是symbol 
      in = in->left ;
      while ( in != NULL && in->token.type != NIL ) {
        if ( IsATOM( in->left->token.type ) == false ) return false ;
        else if ( in->left->token.type != SYMBOL ) return false ;
        in = in->right ; 
      } // while 
    } // else 

    // 再檢查 one-or-more-S-expressions
    bone = bone->right ; // 第二個參數所在bone   
    if ( bone == NULL || bone->token.type == NIL ) return false ; // 一定要1~多個 S-Exp 
    while ( bone != NULL && bone->token.type != NIL ) {
      in = bone->left ; // 第二個參數所在位置
      if ( IsSExp( in ) == false ) return false ;
      bone = bone->right ;
    } // while 

    return true ;
  } // CheckLambdaFormat()

  void CheckArgsType( NodePtr in, NodePtr para, bool & error, string & errormsg ) { // in = (root)
    // check para 的type是否滿足in->str 這個function 
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

  } // CheckArgsType() // ( fn名, 參數)	

  void GetBoundSymbol( NodePtr in, NodePtr & out, bool useLocal ) {
    bool find = false ;
    if ( IsBoundFunction( in->token ) ) { // 是被定義過的function 
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
    } // if 是被定義過的function
    else {
      out = in ; 
      int i = 0 ;
      if ( useLocal ) { // 查找區域變數
                    
        while ( i < glocal.size() ) {
          if ( in->token.str == glocal.at( i ).str ) { 
            out = glocal.at( i ).value ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // 是lambda且為top level 則印<procedure lambda > 
            } // if 

            find = true ;
          } // if 
      
          i++ ;
        } // while

      } // if 

      i = 0 ;
      if ( find == false ) { // 查找全域變數 
        while ( i < gSymbolTAB.size() && find == false ) {
          if ( in->token.str == gSymbolTAB.at( i ).str ) { 
            out = gSymbolTAB.at( i ).value ;
            FunctionType fnType = NONE ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // 是lambda且為top level 則印<procedure lambda > 
            } // if 
            else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
              // 除了老大定義的function跟 "被quote"以外的 top level function (自定義function)
              out->token.funcType = fnType ;
            } // else if 

            find = true ; 
          } // if 
      
          i++ ;
        } // while
      } // if  找全域變數    
    } // else

    if ( find == false ) { // 沒定義此symbol 
      out = in ;
    } // if
  } // GetBoundSymbol()
  
  void Eval( NodePtr in, NodePtr & out, bool & error, string & errormsg, NodePtr &errorNode, 
             bool useLocal ) { // useLocal = 是否使用local variable 
    bool done = false ; // define/ cond/ if/ and / or直接做完就return結果 
    bool customizeDone = false ;
    vector<Symbol> glocalTemp ; // 存舊的glocal，fn結束之後，存回去
    NodePtr copy_tree = NULL ;
    CopyTree( copy_tree, in ); 
    
    if ( IsATOM( in->token.type ) && in->token.type != SYMBOL ) { // ATOM 
      out = in ; // return that atom
      out->left = NULL ;
      out->right = NULL ;
    } // if ATOM
    else if ( in->token.type == SYMBOL ) { // SYMBOL 
      bool find = false ;
      if ( IsBoundFunction( in->token ) ) { // 是被定義過的function 
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
      } // if 是被定義過的function
      else {
        out = in ; 
        int i = 0 ;
        if ( useLocal ) { // 查找區域變數
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // 是lambda且為top level 則印<procedure lambda > 
              } // if 

              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if 

        i = 0 ;
        if ( find == false ) { // 查找全域變數 
          while ( i < gSymbolTAB.size() && find == false ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
              FunctionType fnType = NONE ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // 是lambda且為top level 則印<procedure lambda > 
              } // if 
              else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
                // 除了老大定義的function跟 "被quote"以外的 top level function (自定義function)
                out->token.funcType = fnType ;
              } // else if 

              find = true ; 
            } // if 
      
            i++ ;
          } // while
        } // if  找全域變數    
      } // else

      if ( find == false ) { // 沒定義此symbol 
        out = in ;
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->str + "\n" ;
      } // if
    } // else if SYMBOL
    else { // 此為一個node ( root )
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
        if ( IsBoundFunction( in->left->token ) ) { // 是定義過的Function // 
           
          // 不是TopLevel 且為define clear env.------------------------------------------(A-START)
          // PART_A
          bool isTopLevel = IsTopLevel( in->left ) ;
         
          if ( isTopLevel == false  &&  in->left->token.funcType == DEFINE ) { // (A-1) 
            error = true ;
            errormsg = "ERROR (level of DEFINE)\n" ;        
          } // if 不是TopLevel 且為define
          else if ( isTopLevel == false  && in->left->token.funcType == CLEAN_ENVIRONMENT ) {
            error = true ;
            errormsg = "ERROR (level of CLEAN-ENVIRONMENT)\n" ;
          } // else if 不是TopLevel 且為clear_env. 
          else if ( isTopLevel == false  && in->left->token.str == "exit" ) {
            error = true ;
            errormsg = "ERROR (level of EXIT)\n" ;
          } // else if 不是TopLevel 且為exit.  -----------------------------------------------------(A-1)
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
              errorNode = in ; // error錯的那個Node 如: (define ) 回傳指向最外面的括號() 
              error = true ;
            } // if
            else { // format正確，計算整個tree(從in開始)
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
            if ( IsCustomize( in->left->str ) ) { // 自定義的function 
              if ( CheckCustomizeArgs( in->left->str, in->right ) == false ) {
                error = true ;
                errormsg = "ERROR (incorrect number of arguments) : " ;
                bool temp_error = false ;
                Eval( in->left, errorNode, temp_error, errormsg, errorNode, useLocal ) ; 
              } // if 
              else { // 參數數量正確 
                in->left->token.funcType = CUSTOMIZE ;
                glocalTemp = glocal ;
                // 傳function名，參數(起始)所在的bone
                DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
                NodePtr fnDefine = GetFnDefine( in->left->str ) ; // 取得此function要做的事 
                useLocal = true ;
                
                // eval( Second arg S2 of the main S-exp)
                NodePtr in_args = fnDefine ; // in_args左邊有參數的龍骨  in 為main_S-exp 
                NodePtr out_result = NULL ;
                bool paraCorrect = true ; 
                if ( error ) { // 參數是否有值 
                   paraCorrect = false ;
                } // if 

                while ( in_args != NULL && in_args->token.type != NIL && paraCorrect ) { 
                  // 此node的left有參數
	              if ( error && errormsg == "ERROR (unbound parameter) : " ) {
                    error = false ;
                    errormsg = "" ;	
	              } // if 

                  Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = 參數
                  in_args->left = out_result ; // 接起來
                  if ( error == false ) {
                    CheckArgsType( in, out_result, error, errormsg ) ; // ( fn名(token), 參數)	
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

                  in_args = in_args->right ; // 換下一個龍骨 
                } // while

                out = out_result ; 
                if ( paraCorrect && out == NULL ) {
                  error = true ;
                  errormsg = "ERROR (no return value) : " ;
                  errorNode = copy_tree ;
                } // if 
                /*
                if ( error == false ) { // 無error才繼續做下去 
                  Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // 執行此function
                  if ( error && errormsg == "ERROR (no return value) : " ) errorNode = copy_tree ;
                } // if 
                */               
                customizeDone = true ;
              } // else 
            } // if 
            else { // 非自定義function如: car cdr 
              int numOfArgs = CountNumOfArgs( in->right ) ;
              // Check numof args 是否正確 
              CheckNumOfArgs( in->left->token.funcType, in, numOfArgs, error, errormsg ) ;
            } // else 
                          
          } // else --------------------------------------------------------------------------------(A-4)
          
          // ---------------------------------------------------------------------------------------(A-END)
        } // if 是定義過的Function 
        else { // 此SYMBOL不是被定義過的function名稱 
          if ( IsBoundSymbol( in->left->token ) == false ) { // SYMBOL尚未被定義
            error = true ; 
            errormsg = "ERROR (unbound symbol) : " + in->left->str + "\n" ;
          } // if 
          else { // 已定義此SYMBOL但不是function
             
            NodePtr define_node = GetDefineSymbolNode( in->left, useLocal ) ;
            NodePtr temp_node = NULL ;
            CopyTree( temp_node, define_node ) ; 
            in->left = temp_node ;
            if ( IsProcedureFn( define_node->token ) ) { // 是定義過的"老大指定FN" 以接起來了 所以不用做事 
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
        } // else  此SYMBOL不是被定義過的function名稱 
      } // else if first argument is SYMBOL
      else { // ( (。。。) ... ) 的 (。。。)
        // evaluate (。。。) 也就是in->left(0) 
        NodePtr temp = in ;
        NodePtr tempOut = NULL ;
        if ( in->left != NULL && in->left->left != NULL && 
             in->left->left->token.funcType == LAMBDA ) { // 若為lambda 
          useLocal = true ;
        } // if
        else { // 非lambda的其他 
          Eval( in->left, tempOut, error, errormsg, errorNode, useLocal ) ;
        
          if ( error == false ) {
            in->left = tempOut ; // 接起來 
            // ----------------------------------------------------------------------- PART_B (START) 
            // 應為Fn名稱  若temp本身是ATOM 就不是FN  
            if ( IsATOM( tempOut->token.type ) && IsBoundFunction( tempOut->token ) ) { 
              // ( IsATOM( tempOut->token.type ) == false && IsBoundFunction( tempOut->left->token ) ) 
              int numOfArgs = CountNumOfArgs( temp->right ) ; // in->right為此FN的參數龍骨1 
              // Check numof args 是否正確 
              CheckNumOfArgs( temp->left->token.funcType, temp, numOfArgs, error, errormsg ) ;
              // lambda的errormsg可能要另外處理 或是CheckNumOfArgs中處理 
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
            else { // 非Fn名稱 
              error = true ;
              errormsg = "ERROR (attempt to apply non-function) : " ;
              // errormsg = errormsg + tempOut->str + "\n" ;
              errorNode = tempOut ;  
            } // else
          
            // ---------------------------------------------------------------------------(PART_B) END 
          } // if
        } // else 非lambda的其他
          
        
      } // else ( (。。。) ... ) 的 (。。。)
      
      
      NodePtr out_result = NULL ;
      if ( DoNotNeedEvalARGS( in->left ) || in->left->token.funcType == LAMBDA ) { 
        // 有些指令不需要判斷下一個參數，要直接進行運算 
        ; 
      } // if
      else if ( in->left != NULL && in->left->left != NULL && 
                in->left->left->token.funcType == LAMBDA ) {
        useLocal = true ;
      } // else if 
      else { // 除QUOTE都要繼續判斷下一個參數 
        // eval( Second arg S2 of the main S-exp)
        NodePtr in_args = in->right ; // in_args左邊有參數的龍骨  in 為main_S-exp 
        while ( in_args != NULL && in_args->token.type != NIL ) { // 此node的left有參數
	      if ( error && errormsg == "ERROR (unbound parameter) : " ) {
            error = false ;
            errormsg = "" ;	
	      } // if 

          Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = 參數
          in_args->left = out_result ; // 接起來
          if ( error == false ) {
            CheckArgsType( in, out_result, error, errormsg ) ; // ( fn名(token), 參數)	
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

          in_args = in_args->right ; // 換下一個龍骨 
        } // while
      } // else 
        

      if ( error == false && customizeDone == false ) { 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // 計算結果並回傳
        out = out_result ;
      } // if
      else if ( error && errormsg == "ERROR (unbound parameter) : " ) {
        error = false ;
        errormsg = "" ; 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // 計算結果並回傳
        out = out_result ; 
      } // else if 
      else { // 自定義的執行完 
        glocal = glocalTemp ; // 將舊glocal存回來 也就是將local variables pop掉
      } // else 

      // ---------------------------------------------------------------------------(PART_C) END  
    } // else 
  } // Eval()

  bool IsPair( NodePtr cur, string & errormsg ) { // (cur = root)

    NodePtr fn = cur->left ; // fn 暫存(指向) car, cdr這些function的Node (errormsg需要印出此fn名時用)
    cur = cur->right ; // (1)
    if ( cur->right != NULL && cur->right->token.type != NIL ) { // ex: (car 3 4) error (因為只能有一個參數) 
      errormsg = "ERROR (incorrect number of arguments) : " + fn->str + "\n" ;        
      return false ;
    } // if  ex: (car 3 4) error
    
    // Token define_token = GetDefineToken( cur->left->token ) ; 
    if ( IsATOM( cur->left->token.type ) ) { // ( car 3 ) cur->left 就是 3 這個node 
      errormsg = "ERROR (" + fn->str + " with incorrect argument type) : " ;
      return false ;
    } // if ( car 3 ) cur->left 就是 3 這個node 
    
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

  void BeenQuoted( NodePtr cur ) { // 將cur所指向的樹的參數 消除特殊意義(被quote起來的意思)
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

  void FloatStrToIntStr( string in, string & out ) { // 將float轉乘int(但皆以string的方式儲存) 
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
    // 將cur所指向的樹的所有"參數"相加 
    float sum = 0 ;
    float num = 0 ; // 用來接參數的float(原為string)
    bool isInt = true ; // 紀錄是float的加法還是integer的加法 
    result = new Node() ; // 回傳新的Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // 轉成float再運算
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type不是int也不是float 
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
    // 若是3.0 且要印int 就會直接印出3
    // 若是要印float 印"%.3f"也會是對的答案
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
    // 將cur所指向的樹的所有"參數"相加 
    bool isInt = true ; // 紀錄是float的加法還是integer的加法 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數type不正確 
      error = true ;
      errormsg = "ERROR (- with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // 第一個參數type正確 
      float sum = atof( cur->left->str.c_str() ) ; // 第一個參數 
      if ( cur->left->token.type == FLOAT ) { // 判斷第一個參數的type 
        isInt = false ;
      } // if 
      
      float num = 0 ; // 用來接參數的float(原為string)    
      result = new Node() ; // 回傳新的Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // 換下一個參數 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        num = atof( cur->left->str.c_str() ) ; // 轉成float再運算
        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type不是int也不是float 
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
      // 若是3.0 且要印int 就會直接印出3
      // 若是要印float 印"%.3f"也會是對的答案 
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
    // 將cur所指向的樹的所有"參數"相加 
    float sum = 1 ;
    float num = 0 ; // 用來接參數的float(原為string)
    bool isInt = true ; // 紀錄是float的加法還是integer的加法 
    result = new Node() ; // 回傳新的Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // 轉成float再運算
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type不是int也不是float 
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
    // 若是3.0 且要印int 就會直接印出3
    // 若是要印float 印"%.3f"也會是對的答案 
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
    // 將cur所指向的樹的所有"參數"相加 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數type不正確 
      error = true ;
      errormsg = "ERROR (/ with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // 第一個參數type正確 
      bool isInt = true ; // 紀錄是float的加法還是integer的加法
      // float sum = atof( cur->left->str.c_str() ) ; // 第一個參數 
      double sum ;
      stringstream ss ;
      ss << cur->left->str ;
      ss >> sum ;
      
      
      
      // double sum = atof( cur->left->str.c_str() ) ;
      if ( cur->left->token.type == FLOAT ) { // 判斷第一個參數的type 
        isInt = false ;
      } // if 
      
      // float num = 0 ; // 用來接參數的float(原為string) 
      double num = 0 ;
      result = new Node() ; // 回傳新的Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // 換下一個參數 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        // num = atof( cur->left->str.c_str() ) ; // 轉成float再運算

        stringstream ss ;
        ss << cur->left->str ;
        ss >> num ;



        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type不是int也不是float 
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
      // 若是3.0 且要印int 就會直接印出3
      // 若是要印float 印"%.3f"也會是對的答案 
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
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // 龍骨本人不是NIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ; // 跳出迴圈 
      } // if 
      else if ( temp->token.type == NIL ) { // 找到第一個NIL 就回傳NIL 
        result = temp ;
        toBreak = true ; // 跳出迴圈 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // 代表整個東西都沒有NIL 
      result = lastSExp ; // 則回傳最後一個S-Exp 
    } // if 
  } // And()

  void Or( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal ) { // Or
    NodePtr lastSExp = NULL ;
    bool toBreak = false ;
    NodePtr temp = NULL ;
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // 龍骨本人不是NIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ;
      } // if 
      else if ( temp->token.type != NIL ) {
        result = temp ;
        toBreak = true ; // 跳出迴圈 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // 代表整個東西都是NIL 
      result = lastSExp ; // 則回傳最後一個S-Exp (也就是NIL) 
    } // if 
  } // Or()

  void BiggerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f, 
                   NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (> with incorrect argument type) : " ; 
      errorNode = cur->left ;     
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 第一個參數是數字 
      float f2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (> with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          f2 = atof( cur->left->str.c_str() ) ; // 此參數是數字 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (>= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 第一個參數是數字 
      float f2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (>= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          f2 = atof( cur->left->str.c_str() ) ; // 此參數是數字 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (< with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 第一個參數是數字 
      float f2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (< with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          f2 = atof( cur->left->str.c_str() ) ; // 此參數是數字 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (<= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 第一個參數是數字 
      float f2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (<= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          f2 = atof( cur->left->str.c_str() ) ; // 此參數是數字 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // 第一個參數是數字 
      float f2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          f2 = atof( cur->left->str.c_str() ) ; // 此參數是數字 
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
    
    if ( cur->left->token.type != STRING ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string>? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 第一個參數是數字 
      string s2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string>? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          s2 = cur->left->str ; // 此參數是數字 
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
    if ( cur->left->token.type != STRING ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string<? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 第一個參數是數字 
      string s2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string<? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          s2 = cur->left->str ; // 此參數是數字 
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
    if ( cur->left->token.type != STRING ) { // 第一個參數不是數字 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string=? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // 第一個參數是數字 
      string s2 ;
      cur = cur->right ; // 第二個參數 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // 第二個參數不是數字 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string=? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // 此參數是數字
          s2 = cur->left->str ; // 此參數是數字 
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
    string temp = "" ; // 待加入之string 
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
    NodePtr para1 = cur->left ; // 第一個參數
    cur = cur->right ;
    bool isTrue = false ;
    NodePtr para2 = cur->left ; // 第二個參數 
    if ( para1 == para2 ) {
      isTrue = true ;
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type ) ) { // 都是ATOM 
      if ( para1->str == para2->str ) { // 且相同
        if ( para1->token.type == STRING && para2->token.type == STRING ) { // 若是String則為例外 -> false 
          result = f ;  
        } // if 
        else {
          isTrue = true ;
        } // else 
      } // if 
      else { // 都是ATOM但名字(str)不同 
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
      else { // 其中一棵樹結束了 另一棵尚未結束 
        return f ;
      } // else 
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type )  ) { // 都是ATOM 
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
    else if ( IsATOM( para1->token.type ) == false && IsATOM( para2->token.type ) == false ) { // 都不是ATOM
      NodePtr status = Equal( para1->left, para2->left, t, f ) ;
      if ( status->str == "#t" ) {
        return Equal( para1->right, para2->right, t, f ) ;
      } // if 
      else {
        return f ;
      } // else  
    } // else if  都不是ATOM
    else { // 其中一個是ATOM 另一個不是 
      return f ;
    } // else 其中一個是ATOM 另一個不是

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

      
      CopyTree( temp->left, cur->left ) ; // 往左走 
      CopyTree( temp->right, cur->right ) ; // 往右走 
    } // else
    
  } // CopyTree()

  void If( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal, NodePtr copy_tree ) {
    NodePtr temp = NULL ;
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // 把cur這個樹copy一份到temp中 
    NodePtr tempOut = NULL ;
    temp = temp->right ; // 第一個參數 (計算完應為False(nil) 或 非False)
    Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
    // (計算完應為False(nil) 或 非False)
    if ( error == false ) {
      temp = temp->right ;
      if ( tempOut->token.type != NIL ) { // true
        Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
        if ( error == false ) {
          result = tempOut ;
        } // if no error
      } // if 
      else { // false
        temp = temp->right ; // 第三個參數所在龍骨 
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
    NodePtr bone = NULL ; // 負責走龍骨的部分 
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // 把cur這個樹copy一份到temp中  
    NodePtr tempOut = NULL ;
    temp = temp->right ; // 第一個參數 (第一個if的概念)
    bone = temp ; // 第二個龍骨
    bool done = false ;
    while ( error == false && done == false && bone != NULL && bone->token.type != NIL ) {
      temp = bone->left ; // if/ else if / else 的位置
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // (計算完應為False(nil) 或 非False)
      
      if ( error && ( bone->right == NULL || bone->right->token.type == NIL ) 
           && tempOut != NULL && tempOut->str == "else" ) {
        // 最後一個龍骨 是else   ( 其他else 被判定成 unbound symbol ) 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          error = false ; // 設回false 才能繼續算
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 
 
          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 要回傳的結果 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error        

          temp = temp->right ;
        } // while 
      } // if 最後一個龍骨
      else if ( error == false && ( bone->right == NULL || bone->right->token.type == NIL ) 
                && temp->left->str == "else" ) { 
        // else 被定義過 但此else 是最後一個Sexp 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 

          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 要回傳的結果 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error

          temp = temp->right ;
        } // while 
      } // else if else 被定義過 但此else 是最後一個Sexp  
      
    
      if ( done == false && error == false ) {

        if ( tempOut->token.type != NIL ) { // true
          temp = temp->right ;
          while ( temp != NULL && temp->token.type != NIL && done == false ) {
            if ( error == true && errormsg == "ERROR (no return value) : " ) {
              error = false ;
              errormsg = "" ;
            } // if 

            Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 要回傳的結果 
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

  void DefineSym( NodePtr bone ) { // 定義local variable 
    NodePtr temp = bone->left ;
    NodePtr tempOut = NULL ;
    bool error = false ; 
    string errormsg = "" ;
    NodePtr errorNode = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr temp_bone = bone ; // 暫存初始的bone 
    while ( bone != NULL && bone->token.type != NIL ) { // bone是最大的框框 ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp是裡面的小括號 ( x 5 ) 或( y 3 ) 
      temp = temp->right ;
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 計算該symbol的value
      temp->left = tempOut ;
      bone = bone->right ; // 換下一個參數 
    } // while 
 
    bone = temp_bone ;
    while ( error == false && bone != NULL && bone->token.type != NIL ) { 
      // bone是最大的框框 ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp是裡面的小括號 ( x 5 ) 或( y 3 ) 
      s.str = temp->left->str ; // symbol 
      temp = temp->right ;
      s.value = temp->left ;
      glocal.push_back( s ) ; // 將local variable存入vector中 
      bone = bone->right ; // 換下一個參數 
    } // while 
  } // DefineSym()


  void DeleteDefineFn( string str ) { // 刪去已定義的function 
    // 其他自定義的Function 
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

  void DeleteDefineSym( string str ) { // 刪去已定義的symbol
    // 其他自定義的Function 
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


  int GetFnArgsNum( string fnName, vector<string> & args ) { // 找到fnName這個function後，回傳其參數資訊 
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
 
  string GetOriginFn( string fnName ) { // 找到fnName原本是被誰定義的 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( fnName == gFunctionTAB.at( i ).str ) {
        if ( gFunctionTAB.at( i ).originFn == "" ) { // 是原生function
          return fnName ;
        } // if 
        else { // 是由別人定義的 
          return gFunctionTAB.at( i ).originFn ; 
        } // else 
      } // if 

      i++ ;
    } // while

    return fnName ; 
  } // GetOriginFn()

  int GetProcedureFnArgs( NodePtr f ) { // 找到老大所定義的function，並回傳規定的參數數量 
    if ( f->str == "cons" || f->str == "define" || f->token.funcType == OPERATOR_FN || f->str == "and" || 
         f->str == "or" || f->token.funcType == EQU_TEST ) return 2 ;

    return 1 ; 
  } // GetProcedureFnArgs() 


  void DefineLambdaLocal( NodePtr para1, NodePtr para2, bool & error, string & errormsg, 
                          NodePtr & errorNode ) { // 定義lambda的參數值 
    int i = 0 ;

    NodePtr tempOut = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr p2 = para2 ; // 暫存para2一開始的位置 
    
    // 全部都計算好要被定義的值後，才存入glocal中 
    // 以下while 負責計算所有要被定義的值 
    while ( para2 != NULL && para2->token.type != NIL && error == false ) { 
      Eval( para2->left, tempOut, error, errormsg, errorNode, useLocal ) ; // 計算該symbol的value
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
        if ( useLocal ) { // 查找區域變數
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if

        i = 0 ; 
        if ( find == false ) { // 查找全域變數 
          while ( i < gSymbolTAB.size() ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
            } // if 
      
            i++ ;
          } // while
        } // if
      } // if
      else { // 並非定義的symbol 
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->token.str + "\n" ;
      } // else 
    } // else if 
    else if ( in->left->token.funcType == CONS ) { // CONS (cons, list)  
      if ( in->left->token.str == "cons" ) { // cons
        in = in->right ;
        NodePtr temp = new Node() ;
        temp->left = in->left ; // 第一個參數 放左子樹
        in = in->right ;
        temp->right = in->left ; // 第二個參數 放右子樹
        if ( IsATOM( in->left->token.type ) && in->left->token.type != NIL ) { // 若後面接ATOM 要DOT 
          temp->str = "." ; // 中間記得放點 
          temp->token.type = DOT ;
        } // if 
        else { // 若後面是LIST 就不要加DOT 
          temp->str = "(" ; // 中間記得放點 
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
      BeenQuoted( out ) ; // 將其所指向的樹變成"" (就是那個東西)/ 消除特殊意義 
    } // else if  QUOTE_FN
    else if ( in->left->token.funcType == DEFINE ) { // DEFINE
      out = new Node() ;
      out->left = NULL ;
      out->right = NULL ;
      in = in->right ;
      if ( in->left->token.type == SYMBOL ) { // is SYMBOL
        string symbol = in->left->str ; // symbol 
        out->str = symbol + " defined" ;
        NodePtr para1 = in->left ; // 第一個參數本人 b 
        NodePtr para2 = in->right->left ; // 第二個參數本人 a  
        in = in->right ;
        NodePtr temp = NULL ;
        if ( IsBoundFunction( para2->token ) ) { // ( define b a ) 且a是自定義的function 
          out->token.type = STRING ;
          out->token.funcType = CUSTOMIZE ; 
          Fn f ;
          if ( IsBoundFunction( para1->token ) ) { // 若b為已被定義的funciton，則刪掉重新定義
            DeleteDefineFn( para1->str ) ; // 刪掉之前定義的function 
          } // if
          else if ( IsBoundSymbol( para1->token ) ) {
            DeleteDefineSym( para1->str ) ; // 刪掉之前定義的function 
          } // else if 

          f.str = para1->str ;
          if ( IsProcedureFn( para2->token ) ) { // 是老大定義的function 
            Symbol temps ;
            temps.str = f.str ;
            temps.value = para2 ;
            gSymbolTAB.push_back( temps ) ;

          } // if
          else { // 是我自己定義的function 
            f.value = GetFnDefine( para2->str ) ;
            f.originFn = GetOriginFn( para2->str ) ; // 找到這是由哪個function所定義的 
            f.numOfArgs = GetFnArgsNum( para2->str, f.args ) ; 
            gFunctionTAB.push_back( f ) ;  // 將定義好的function也放入gFunctionTAB中
          } // else 

                            
        } // if // ( define b a ) 且a是自定義的function 
        else { // 是要定義的symbol 
          Symbol s ;
          s.str = symbol ;
          int index = 0 ;
          if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
            s.value = in->left ;                      
          } // if 是lambda 
          else { // 不是lambda                     
            Eval( in->left, temp, error, errormsg, errorNode, useLocal ) ; // Symbol要被定義的值
            out->token.type = STRING ;
            out->token.funcType = CUSTOMIZE ; 
            if ( error == false ) {          
              s.value = temp ;       
            } // if           
          } // else 不是lambda 

          if ( HasExistInSymbolTAB( s.str, index ) ) { // 已經存在在gSymbolTAB中了
            gSymbolTAB.insert( gSymbolTAB.begin() + index, s ) ; // 先放進去 
            gSymbolTAB.erase( gSymbolTAB.begin() + index + 1 ) ; // 刪掉原本的 
          } // if 
          else { // 之前沒出現過， 所以插入最後面 
            gSymbolTAB.push_back( s ) ; 
          } // else 
        } // else 是要定義的symbol 
      } // if SYMBOL
      else { // define (F x ) ( s-exp )
        NodePtr bone = in ;
        in = in->left ; // (F  x y z)
        Fn temp_Fn ;
        int counter = 0 ; // 計算參數數量用 
        if ( IsBoundFunction( in->left->token ) ) { // 若為已被定義的funciton，則刪掉重新定義 
          DeleteDefineFn( in->left->str ) ; // 刪掉之前定義的function 
        } // if 

        temp_Fn.str = in->left->str ; // Fn name
        out->str = temp_Fn.str + " defined" ;
        out->token.type = STRING ;
        out->token.funcType = CUSTOMIZE ;
        if ( gVerbose == false ) { // verbose是false 就不要印 
          out = NULL ;
        } // if
 
        in = in->right ;
        while ( in != NULL && in->token.type != NIL ) { // 將所有function的參數都存起來 
          counter++ ;
          temp_Fn.args.push_back( in->left->str ) ;
          in = in->right ;
        } // while 

        temp_Fn.numOfArgs = counter ;
        in = bone->right ; // 換到Fn的定義(要做什麼事)
        temp_Fn.value = in ;
        temp_Fn.value->token.funcType = CUSTOMIZE ;
        temp_Fn.originFn = temp_Fn.str ; // 是原生function 
        gFunctionTAB.push_back( temp_Fn ) ; 
      } // else define (F x ) ( s-exp )
    } // else if DEFINE
    else if ( in->left->token.funcType == PART_ACCESSOR ) { // PART_ACCESSOR(car, cdr)
      if ( IsPair( in, errormsg ) ) {
        NodePtr temp = in->right->left ; // list放在的位置(10)
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
      t->token.type = T ; // 非常重要 一定要設定type
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
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) ) { // true
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
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
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == NIL ) { // true
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false
      } // else if null?
      else if ( in->left->token.str == "integer?" ) { // integer?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == INT ) { // true
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false
      } // else if  integer?
      else if ( in->left->token.str == "real?" || in->left->token.str == "number?" ) { // real? number?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == INT || temp->token.type == FLOAT ) ) {
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false        
      } // else if real? number?
      else if ( in->left->token.str == "string?" ) { // string?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == STRING ) { // true
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false
      } // else if  string?
      else if ( in->left->token.str == "boolean?" ) { // boolean?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // 此參數有錯誤 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == T || temp->token.type == NIL ) ) {
          out = t ;
        } // else if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false
      } // else if  boolean?
      else if ( in->left->token.str == "symbol?"  ) { // symbol?
        in = in->right ; // (1)
        temp = in->left ;
        // if ( temp->token.type == SYMBOL || temp->token.funcType == BEEN_QUOTE ) { 
        // 因為'3 不是symbol唷 
        if ( temp->token.type == SYMBOL ) {
          out = t ;
        } // if true
        else { // 無錯誤且為false 
          out = f ;
        } // else false
      } // else if symbol?
    } // else if PRIMITIVE_PREDICATE
    else if ( in->left->token.funcType == OPERATOR_FN ) { // OPERATOR_FN(+-*/)
      NodePtr t = new Node() ;
      t->str = "#t" ;
      t->token.type = T ; // 非常重要 一定要設定type
      t->left = NULL ;
      t->right = NULL ;
      NodePtr f = new Node() ;
      f->str = "nil" ;
      f->token.type = NIL ;
      f->left = NULL ;
      f->right = NULL ;

      // Token define_token = GetDefineToken( in->left->token ) ;
       
      if ( in->left->str == "+" ) { // + Add
        Add( in->right, out, error, errormsg, errorNode ) ; // 把參數都加起來 若有錯，也會在其中設定好errormsg
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
      t->token.type = T ; // 非常重要 一定要設定type
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
        NodePtr para1 = in->left ; // 第一個參數
        in = in->right ; 
        NodePtr para2 = in->left ; // 第二個參數

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
      bool useLocal = true ; // 是否要用local variables 
      glocalTemp = glocal ;
      DefineSym( bone->left ) ; // 定義local variables
      bone = bone->right ;
      while ( bone != NULL && bone->token.type != NIL ) { // 開始執行要做的事 
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
        out->token.type = T ; // 非常重要 一定要設定type
        out->left = NULL ;
        out->right = NULL ;
      } // else 
    } // else if
    else if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
      // (( lambda XXXXX ...
      NodePtr root = in ; 
      NodePtr para2 = in->right ; // 要被定義的參數值 會被放到para2(para2為這些值的所在bone) 
      NodePtr para1 = in->left->right->left ; // 參數的bone

      glocalTemp = glocal ;
      DefineLambdaLocal( para1, para2, error, errormsg, errorNode ) ;
      if ( error && errormsg == "ERROR (incorrect number of arguments) : " ) {
        errorNode = in->left->left ;
      } // if 

      in = root->left->right->right ; // lambda的最後一個參數 也就是要執行的地方 
      while ( in != NULL && error == false ) {
        Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
        in = in->right ;
      } // while 

      glocal = glocalTemp ; // 執行完lambda 就將glocal復原 
    } // else if
    else if ( in->left->token.funcType == CUSTOMIZE ) {
      DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
      // 傳function名，參數(起始)所在的bone
      NodePtr fnDefine = GetFnDefine( in->left->str ) ; // 取得此function要做的事 
      useLocal = true ;
      if ( error == false ) { 
        Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // 執行此function 
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
    string str = "" ; // 為了轉成string存進(vector)gLine中 
    bool exit = false ;
    bool error = false ; // 切Token或是文法判斷時，有無錯誤ERROR
    string errormsg = "" ; 
    bool isComplete = true ; // 是否為完整可以建樹的東西 
    while ( exit == false && scanf( "%c", &ch ) != EOF  ) { // 若還沒讀完

      if ( ch == '\n' ) { // 換行也要存進去 為了判斷ERROR是在第幾行 
        str = str + ch ; // 轉成string存 
        gLine.push_back( str ) ;
        str = "" ;
        GetToken( error ) ; // 去切token，分類
        gLine.clear() ; 

        // 出來後 整行分類完了
        // 去文法判斷 ，若整行還不夠就等讀下一行 
        error = false ;
        int index = 0 ; // gToken的index
        isComplete = true ;
        bool isFirstToken = true ;
        bool isEOF = false ;
        bool hasQuote = false ;
        SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

        if ( error ) { // 文法錯誤印ERROR 
          cout << errormsg ;
          int k = 0 ;
          while ( ! gToken.empty() && k <= index ) { // 清到錯的地方   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

            
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // 清掉這一行 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
            
          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // 清掉換行 

          gLine.clear() ;
          cout << "\n> " ; 
        } // if 
        else { // 文法無誤 
          if ( IsExitToken() ) exit = true ;
            
          gTree = NULL ;
          NodePtr tree = gTree ;
          int i = 0 ; // index of token
          bool isLeftChild = false ;
          bool isRightChild = false ;
          int lastIndexOfToken = -1 ; // 記建樹的gToken到哪個
          int lp = 0 ; // 左括號數
          int rp = 0 ; // 右括號數 當左右括號數對稱，則表示成功建樹
          string parent = "" ;
          int limitIndex = index ; // 建樹只能建到這裡
          bool useLocal = false ; // 是否使用local Variables 
          if ( isComplete ) {
            if ( hasQuote ) { // 有Quote才需要在Quote前後加左括號 
              int count = SortOutQuote( limitIndex ) ;
              limitIndex = count + limitIndex ;
            } // if 
            
            BuildTree( tree, i, isLeftChild, isRightChild, lastIndexOfToken, lp, rp, 
                       parent, limitIndex ) ;
            // PrettyPrint( gTree, error ) ;
            tree = NULL ;               
            tree = gTree ;
            NodePtr out = NULL ;
            NodePtr errorNode  = NULL ; // 有錯誤的要印s-exp，存在這裡 
            // result(pointer)指向要印的東西(EVAL的結果) 
            Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
            if ( IsExitTree( tree ) ) {
              exit = true ; // 結束 
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
            DeleteGToken( limitIndex ) ; // 清掉一部分的gToken
            delete gTree ;
            gTree = NULL ;
            glocal.clear() ;
            if ( exit == true ) ;
            else cout << "\n> " ;  
          } // if          
        } // else 文法無誤            

        error = false ; // 更新error (init)
        errormsg = "" ;     
      } // if       
      else {
        str = str + ch ; // 轉成string存 
        gLine.push_back( str ) ;
        str = "" ;  
      } // else 
    } // while 


    /*
    if ( gLine.size() != 0 && gLine.at( gLine.size() - 1 ) != "\n" ) {

      GetToken( error ) ; // 去切token，分類

      // 出來後 整行分類完了
      // 去文法判斷 ，若整行還不夠就等讀下一行 
      if ( error ) {
        gToken.clear() ;
        cout << "\n> " ;
      } // if
    } // if  
    */
    
    GetToken( error ) ; // 去切token，分類
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
      int index = 0 ; // gToken的index 
      isComplete = true ;
      error = false ;
      bool isFirstToken = true ;
      bool isEOF = false ;
      bool hasQuote = false ; 
      SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

      if ( error ) { // 文法錯誤印ERROR  
        if ( isEOF ) { // 結束 
          gToken.clear() ; 
        } // if 
        else {
          cout << errormsg ;
          int k = 0 ;
        
          while ( k <= index ) { // 清到錯的地方   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

          
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // 清掉這一行 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
          

          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // 清掉換行 

          cout << "\n> " ;
          errormsg = "" ;
        } // else 
      } // if 文法錯誤印ERROR 
      else { // 文法無誤
        if ( IsExitToken() ) end = true ;
        
        if ( end == true ) gToken.erase( gToken.end() ) ;
        gTree = NULL ;
        NodePtr tree = gTree ; 
        int i = 0 ; // index of token
        bool isLeftChild = false ;
        bool isRightChild = false ;
        int lastIndexOfToken = -1 ; // 記建樹的gToken到哪個
        int lp = 0 ; // 左括號數
        int rp = 0 ; // 右括號數 當左右括號數對稱，則表示成功建樹
        string parent = "" ;
        int limitIndex = index ; // 建樹只能建到這裡
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
          NodePtr errorNode  = NULL ; // 有錯誤的要印s-exp，存在這裡 
          // result(pointer)指向要印的東西(EVAL的結果) 
          Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
          if ( IsExitTree( tree ) ) {
            exit = true ; // 結束 
          } // if 
          else if ( error && errorNode == NULL ) cout << errormsg ;
          else if ( error ) {
            cout << errormsg ;
            // 因為non-function 要印出#<Procedure> 這行 所以設定成no error 有點不確定! 
            if ( errormsg == "ERROR (attempt to apply non-function) : " ) error = false ;
            PrettyPrint( errorNode, error ) ;  
          } // else if 
          else {
            PrettyPrint( out, error ) ;
          } // else 

          if ( isFirstToken ) limitIndex-- ; 
          DeleteGToken( limitIndex ) ; // 清掉一部分的gToken
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
