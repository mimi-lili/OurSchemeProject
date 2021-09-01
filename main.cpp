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
  string str ; // �L���H
  bool error ; // �O�_���~ 
  string errormsg ; // ���~�T�� 
  TerminalToken type ; // ����
  FunctionType funcType ; // �禡���� 
  int lastIndexOfLine ; // �b(vector)gLine����index(�s�̫�@��char�Ҧb��m) 
}; 

struct Node{
  Token token ;
  string str ; // �L���H
  Node * left ; // ��pointer 
  Node * right ; // �kpointer
  int ans_int ;
  float ans_float ; // �Ofloat�N������float�B�� 
};

typedef Node * NodePtr ;

struct Symbol{
  string str ; // symbol���W��
  Node * value ; // Symbol�n�Q�w�q���� 
};
struct Fn{
  string str ; // Function�W��
  int numOfArgs ; // ���h�ְѼ�
  string originFn ; // �o�O�Q����functionn�ҩw�q�� 
  vector<string> args ; // �Ѽ�(�W��)�s�b�o�Ӹ̭� 
  Node * value ; // ��fn���w�q(�n������)
};



int gTestNum = 0 ;
bool gVerbose = true ; // �S��n��@��...��define clean evn.�n���n�L���� 
vector<string> gLine ; // �sŪ�J���F��(�򥻤W�O�@��) 
vector<Token> gToken ; // �n��Token��Token��
vector<Symbol> gSymbolTAB ; // symbol Table �w�q�L��symbol���|�s�b�o�� 
vector<Fn> gFunctionTAB ; // �w�q�L��function���s�b�o�� ���Ѭd�ߥ� 
vector<Symbol> glocal ; // let�|�Ψ쪺local variable���צs�b�o�� 


NodePtr gTree ; // ��𥻾� 

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

  bool IsEscape( int index ) { // ' " \ �o�T��case�ݭnescape (\n, \t�d�ۧY�i)   
    if ( gLine.at( index ) == "'"  || gLine.at( index ) == "\"" || gLine.at( index ) == "\\"
         || gLine.at( index ) == "n"  || gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscape()

  bool IsEscapeN( int index ) { // ' " \ �o�T��case�ݭnescape (\n, \t�d�ۧY�i)   
    if ( gLine.at( index ) == "n" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeN()

  bool IsEscapeT( int index ) { // ' " \ �o�T��case�ݭnescape (\n, \t�d�ۧY�i)   
    if ( gLine.at( index ) == "t" ) {
      return true ;
    } // if 

    return false ; 
  } // IsEscapeT()

  bool IsNumber( string temp ) { // �Ѧ�https://www.itread01.com/content/1546245258.html 
    char ch ;
    stringstream ss( temp ) ;
    double isDouble ;
    
    if ( ! ( ss >> isDouble ) ) { // ss >> isDouble�� ss�ഫ��double���ܼ�
      return false ; // (int/ float����)�A�Ǵ����ѫh�Ȭ�0 
    } // if 
    
    if ( ss >> ch ) { // �˴����~��J(�Ʀr�[�r��)(�Ҧp�G12.a�^
      return false ; // ���ɱ���.a������ (���󦨥ߡA�ҥHreturn false) 
    } // if 

    return true ;
  } // IsNumber()
  
  int CountEnter( int index ) { // ��ثe�|���B�z���F�褤���X�Ӵ��� 
    int counter = 0 ;
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gToken.size() && toBreak == false ) {
      if ( gToken.at( i ).str == "\n" ) {
        counter++ ;
      } // if

      if ( i == index ) { // �J�쥻�H �[ 1(����)�õ��� 
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
      if ( temp == "\n" ) { // ���� 
        gtemp.type = ENTER ;
      } // if 
      else if ( temp == "(" ) { // ���A�� 
        gtemp.type = LEFT_PAREN ;
        int j = indexOfLine + 1 ;
        while ( j < gLine.size() && ( gLine.at( j ) == " " || gLine.at( j ) == "\t" ) ) j++ ; 
        if ( j < gLine.size() && gLine.at( j ) == ")" ) { // () nil�����p 
          gtemp.type = NIL ;
          gtemp.str = "nil" ;
          indexOfLine = j ;
          gtemp.lastIndexOfLine = j ;
        } // if 
      } // else if (���A��) 
      else if ( temp == ")" ) { // �k�A�� 
        gtemp.type = RIGHT_PAREN ;
      } // else if (�k�A��)
      else if ( temp == "'" ) { // (��)�޸� Quote 
        gtemp.type = QUOTE ;
        gtemp.funcType = QUOTE_FN ;
      } // else if (��)�޸� Quote
      else if ( temp == "nil" || temp == "#f" ) { // NIL
        gtemp.type = NIL ;
      } // else if (nil)
      else if ( temp == "t" || temp == "#t"  ) { // T
        gtemp.str = "#t" ;
        gtemp.type = T ;
      } // else if (T)
      else if ( temp == ";" ) { // ����
        gtemp.type = COMMENT ;  
        while ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) != "\n" ) { // Ū�촫�� 
          indexOfLine++ ;
        } // while

        if ( indexOfLine < gLine.size() ) 
          indexOfLine-- ; // �^�h��|++, �A���^�촫��    
      } // else if (����)
      else if ( temp == "\"" ) { // String 
        // string lastStr = "" ; // �s�W�@�ӹJ�쪺TOKEN
        bool toBreak = false ; 
        gtemp.str = temp ; // "(��)�޸�            
        indexOfLine++ ;
        while ( indexOfLine < gLine.size() && toBreak == false ) { // ����Ū��ĤG��(��)�޸� 
          if ( gLine.at( indexOfLine ) == "\"" ) { // �r�굲�����k�޸� 
            toBreak = true ;
          } // if 
          else if ( gLine.at( indexOfLine ) == "\\" ) { // �ϱ׽u 
            if ( indexOfLine + 1 >= gLine.size() ) toBreak = true ;
            else if ( IsEscape( indexOfLine + 1 ) ) { // �P�_���ϱ׽u�O�_��escape�N�q 
              indexOfLine++ ; // ���L��escape�w�q���ϱ׽u  �p: �ثe�� \" �� "  
            } // else if 

            char enter = '\n' ; // ����
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
          else { // string������ 
            gtemp.str = gtemp.str + gLine.at( indexOfLine ) ;
            indexOfLine++ ;
          } // else            
        } // while 
      
        if ( indexOfLine < gLine.size() && gLine.at( indexOfLine ) == "\"" ) { // STRING�榡���T       
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
      else if ( IsNumber( temp ) ) { // �Ʀr(int/float) 

        if ( temp[0] == '+' ) {
          temp.erase( 0, 1 ) ; // �qindex = 0�}�l�A�R��1��char
        } // if 


        gtemp.str = "" ;        
        bool isFloat = false ;
        int i = 0 ;
        int indexOfDot = 0 ;
        char lastCH = '\0' ;
        while ( i < temp.length() ) { // �P�_�Oint�٬Ofloat
 
          if ( temp[i] == '.' ) {
            indexOfDot = i ;
            isFloat = true ;
            if ( lastCH == '-' ) {
              gtemp.str = gtemp.str + "0" ;
            } // else if

            if ( i == 0 ) { // �Y�O.1 => 0.1 �e���ɹs 
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
          int num = temp.length() - indexOfDot - 1 ; // �᭱���X���
          int n = 3 - num ; // �n�ɵ���0
          while ( n > 0 ) {
            gtemp.str = gtemp.str + "0" ;
            n-- ;
          } // while
    
        } // if  
      } // else if (�Ʀr(int/float))
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

      if ( error ) { // �Ystring�� �h�̫᭱�]�n��ENTER 
        gtemp.str = "\n" ; 
        gtemp.type = ENTER ;
        gToken.push_back( gtemp ) ;
      } // if 
    } // else 
  } // Classify()
    
  void GetToken( bool &error ) { // ���� => Token��  
    bool toBreak = false ;
    int indexOfLine = 0 ; // index of (vector) gLine(string)
    int indexOfToken = 0 ; // index of (vextor) gToken(Token)
    int countEnter = 0 ; // �⦳�X�Ӵ��� 
    string temp = "" ;
    while ( toBreak == false && indexOfLine < gLine.size() ) { // �٨S������ 

      if ( gLine.at( indexOfLine ) == "\n" ) { // ����
        if ( temp != "" ) { // �e����Token�n�B�z 
          indexOfToken++ ; // ���U�@��TOKEN 
          indexOfLine-- ; // ���^Token���a�� 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // �^�촫��o�� 
          temp = "" ;
        } // if  


 
        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // ���U�@��TOKEN 
        Classify( temp, indexOfLine, indexOfToken, error ) ;
        temp = "" ; 
 
        toBreak = true ;
      } // if (����) 
      else if ( IsSeparator( gLine.at( indexOfLine ) ) ) { // Separator
        if ( temp != "" ) { // �e����Token�n�B�z 
          indexOfToken++ ; // ���U�@��TOKEN
          indexOfLine-- ; // ���^Token���a�� 
          Classify( temp, indexOfLine, indexOfToken, error ) ;
          indexOfLine++ ; // �^�촫��o��
          temp = "" ;
        } // if 

        temp = gLine.at( indexOfLine ) ;
        indexOfToken++ ; // ���U�@��TOKEN 
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
      indexOfToken++ ; // ���U�@��TOKEN 
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
                       bool & isFirstToken, bool & isEOF, bool & hasQuote ) { // ��k���R 


    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // ����
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 


      index++ ;

    } // while ����

    if ( index < gToken.size() && gToken.at( index ).error == true ) { // �Y�Ostring���~ 
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

    if ( error == true || index >= gToken.size() ) { // ����!�άO��������
      if ( index >= gToken.size() ) isComplete = false ;

      return true ; 
    } // if

    
    while ( index < gToken.size() && gToken.at( index ).type == ENTER ) { // ����ε��� 
      if ( index + 1 == gToken.size() ) {
        isComplete = false ;
        return true ;
      } // if 

      index++ ;

    } // while ����

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
        // ��k����������
        error = true ;
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( index < gToken.size() && gToken.at( index ).type == LEFT_PAREN ) { 
      // ���A�� <S-exp>{<S-exp>}[DOT<S-exp>]�k�A��
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
      if ( j < gToken.size() && gToken.at( j ).type == RIGHT_PAREN ) { // ()�����p 
        gToken.at( index - 1 ).str = "nil" ;
        gToken.at( index - 1 ).type = NIL ;
        gToken.at( index - 1 ).lastIndexOfLine = j ;
        int k = index ;
        while ( k <= j ) { // ���� 
          gToken.erase( gToken.begin() + index ) ;
          k++ ;
        } // while 

        index = index - 1 ;
        isNIL = true ;
        return true ;
      } // if ()�����p 
      else if ( ! SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ) 
        return false ;

      if ( isNIL == false )
        index++ ; // �P�_�U�@�� 
      while ( index < gToken.size() && gToken.at( index ).type != DOT 
              && gToken.at( index ).type != RIGHT_PAREN ) { // ��<S-exp> �h�P�_<S-exp>�O�_���T
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
      else if ( index < gToken.size() && gToken.at( index ).type == DOT ) { // DOT�᭱�u�౵ ATOM/ ���A�� / ��޸� 
        index++ ;
        if ( index >= gToken.size() ) {
          isComplete = false ;
          return true ;
        } // if 

        if ( index < gToken.size() && IsATOM( gToken.at( index ).type ) == false && 
             gToken.at( index ).type != LEFT_PAREN 
             &&  gToken.at( index ).type != QUOTE && gToken.at( index ).type != ENTER && 
             gToken.at( index ).type != COMMENT ) {
        // �YDOT�ᤣ�OATOM / ���A�� / ��޸� / ���� / ���� 
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
          else index++ ; // ���U�P�_

          if ( index < gToken.size() && gToken.at( index ).type == ENTER ) index++ ; 
        } // if 
        //  DOT�᭱�� ATOM/ ���A�� / ��޸� �h�h�P�_�Ӧ��k 
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
        else if ( index < gToken.size() && gToken.at( index ).type != RIGHT_PAREN ) { // DOT<S-exp>��S���k�A��
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
      } // else if // DOT�᭱�u�౵ ATOM/ ���A�� / ��޸�
      else if ( index < gToken.size() && gToken.at( index ).type == RIGHT_PAREN ) { // �k�A�� ->����
        isComplete = true ; 
        return true ;
      } // else if �k�A�� ->����
      else if ( isNIL == false ) return false ;

    } // else if ���A��
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
                  int & LP, int & RP, string parent, int limitIndex ) { // �ؾ� ���k�A���ƹ�١A�h��ܦ��\�ؾ� 

    if ( i >= gToken.size() || i > limitIndex ) { // ����
      ;
    } // if ����
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
    else if ( i < gToken.size() && gToken.at( i ).type == RIGHT_PAREN ) { // �J��k�A��
      RP++ ; // �k�A���� + 1
      lastIndexOfToken = i ;
      if ( isLeftChild ) {
        i++ ;
        BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // if
      else {
        // i++ ;
        // BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
      } // else  
    } // else if �J��k�A��
    else {
      if ( IsATOM( gToken.at( i ).type ) ) { // �J��ATOM
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->token = gToken.at( i ) ;
          tree->str = gToken.at( i ).str ;
          tree->left = NULL ;
          tree->right = NULL ; 
        } // if  
        else if ( isLeftChild ) { // �O���p��
          parent = tree->str ; 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;
        } // if ���p��
        else if ( isRightChild ) { // �O�k�p��
          parent = tree->str ;
          if ( parent == "." && gToken.at( i ).type == NIL ) { // �Y�� ".nil" �h�����I(�o���л\�Y�i
            tree->str = "MimiNote:DontNeedToPutTwoSpace" ;     
          } // if 

          tree->right = new Node() ;
          tree = tree->right ;
          tree->left = NULL ;
          tree->right = NULL ;
          if ( parent == "." ) { // ��DOT�N������
            tree->str = gToken.at( i ).str ;
            tree->token = gToken.at( i ) ;
          } // if 
          else { // �SDOT ->�h��@NODE
            tree->str = "" ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �����h��@node
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �X�ӦA���k 
          } // else         
        } // else if �k�p��

        lastIndexOfToken = i ;
      } // else if  �J��ATOM
      else if ( gToken.at( i ).type == DOT ) { // �J���I
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
          i++ ; // �u���k��
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ;
          lastIndexOfToken = i ;
          int index = i + 1 ;
          bool toBreak = false ;
          while ( gToken.at( index ).str == "\n" ) { // ���泣���L 
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
      
      } // else if �J���I
      else if ( gToken.at( i ).type == QUOTE ) { // �J��QUOTE

        if ( isLeftChild ) { // �u�|�O���p�� 
          tree->left = new Node() ;
          tree = tree->left ;
          tree->str = "quote" ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ;

        } // if 

      } // else if �J��QUOTE
      else if ( gToken.at( i ).type == LEFT_PAREN ) { // �J�쥪�A��
        // bool rightChildDone = true ;
        LP++ ; // �⥪�A���ƶq
        bool done = false ;
        if ( gTree == NULL ) {
          gTree = new Node() ;
          tree = gTree ;
          tree->str = gToken.at( i ).str ;
          tree->token = gToken.at( i ) ;
          tree->left = NULL ;
          tree->right = NULL ; 

          // �u�n�O���A�� �᭱���o�˰�(��n���� �������� �A���k)
          if ( done == false ) {
            tree->str = "(" ;
            tree->left = NULL ;
            tree->right = NULL ;
            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ��������
            lastIndexOfToken = i ;
          } // if

          i++ ;
          parent = tree->str ;
          BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �A���k��
          lastIndexOfToken = i ;
        } // if 
        else {
 
          parent = tree->str ;
          if ( parent == "." ) { // �@�w�O�k�p�� ���O���ݭn�h�@��NODE
            tree->str = "MimiNote:NeedToPutTwoSpace" ; // �Y�� ".(" �h�����I(�o���л\�Y�i
            done = true ; 
            // �u�n�O���A�� �᭱���o�˰�(��n���� �������� �A���k)
            if ( done == false ) {
              tree->str = "(" ;
              tree->left = NULL ;
              tree->right = NULL ;
              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ��������
              lastIndexOfToken = i ;
            } // if

            i++ ;
            parent = tree->str ;
            BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �A���k��
            lastIndexOfToken = i ;


          } // if 
          else { // parent���O�I
            if ( isRightChild ) { // �k�p�ĭn�h���@�B(�h�ؤ@node)
              parent = tree->str ;
              tree->right = new Node() ;
              tree = tree->right ;
              tree->str = "" ;
              tree->left = NULL ;
              tree->right = NULL ;


              // �u�n�O���A�� �᭱���o�˰�(��n���� �������� �A���k)
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                // i++ ;
                parent = tree->str ; 
                // if ( i + 1 < gToken.size() && gToken.at( i+1 ).type == QUOTE ) i++ ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ��������
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �A���k��
              lastIndexOfToken = i ;


            } // if �k�p��
            else { // ���p�� 

              parent = tree->str ;
              tree->left = new Node() ; // ���k�p�ĳ��n������(if parent���O�I)
              tree->right = NULL ;
              tree = tree->left ;
              if ( done == false ) {
                tree->str = "(" ;
                tree->left = NULL ;
                tree->right = NULL ;
                i++ ;
                parent = tree->str ;
                BuildTree( tree, i, true, false, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // ��������
                lastIndexOfToken = i ;
              } // if

              i++ ;
              parent = tree->str ;
              BuildTree( tree, i, false, true, lastIndexOfToken, LP, RP, parent, limitIndex ) ; // �A���k��
              lastIndexOfToken = i ;
            } // else ���p�� 
          } // else parent���O�I         
        } // else 
      

      } // else if �J�쥪�A��
    } // else 
  } // BuildTree()

  int SortOutQuote( int limitIndex ) { // �Y��quote �N�Nquote���e�᳣�[�A�� 'a b = ('a ) b 
    int i = 0 ; // index of gToken
    stack<int> indexToInsertLP ; // ��LP���a�� 
    Token gtempRP ; // RP
    gtempRP.str = ")" ; 
    gtempRP.error = false ;
    gtempRP.type = RIGHT_PAREN ;
    Token gtempLP ; // LP
    gtempLP.str = "(" ;
    gtempLP.error = false ;
    gtempLP.type = LEFT_PAREN ; 
    int count = 0 ; // �p��[�F�h�֪F��
 
    while ( i < gToken.size() && i <= limitIndex ) {
      if ( gToken.at( i ).type == QUOTE && gToken.at( i ).str == "'" ) { // quote���a��N�O��LP���a�� 
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
        else if ( IsATOM( gToken.at( i ).type ) ) { // quote��OATOM 
          int j = i + 1 ;
          gtempRP.lastIndexOfLine = gToken.at( i ).lastIndexOfLine ;
          if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
          else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RP�bATOM�� 
          count++ ; 
        } // else if quote��OATOM
        else if ( gToken.at( i ).type == LEFT_PAREN ) { // quote�᭱�O���A��  
          int j = i + 1 ;
          int lp = 1 ; // ���A���ƶq 
          int rp = 0 ; // �k�A���ƶq 
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
              gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP�b(�w��٪�)�k�A���� 
              count++ ;
              toBreak = true ;
            } // if 

            j++ ; 
          } // while 
        } // else if quote�᭱�O���A�� 
        else if ( gToken.at( i ).type == QUOTE ) { // �s��QUOTE 
          int k = i ;
          while ( k < gToken.size() && gToken.at( k ).type == QUOTE ) { // �]�줣�OQUOTE�Y�i 
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
            if ( IsATOM( gToken.at( k ).type ) ) { // quote��OATOM 
              int j = k + 1 ;
              gtempRP.lastIndexOfLine = gToken.at( j ).lastIndexOfLine ;
              if ( j >= gToken.size() ) gToken.push_back( gtempRP ) ;
              else gToken.insert( gToken.begin()+j, gtempRP ) ; // insert RP�bATOM��
              count++ ; 
            } // if quote��OATOM
            else if ( gToken.at( k ).type == LEFT_PAREN ) { // quote�᭱�O���A��  
              int j = k + 1 ;
              int lp = 1 ; // ���A���ƶq 
              int rp = 0 ; // �k�A���ƶq 
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
                  gToken.insert( gToken.begin()+j+1, gtempRP ) ; // insert RP�b(�w��٪�)�k�A����
                  count++ ; 
                  toBreak = true ;
                } // if 
 
                j++ ; 
              } // while 
            } // else if quote�᭱�O���A�� 
          } // else 
        } // else if �s��QUOTE 

        i-- ; // �^��U�@��QUOTE(���U�|i++) 
      } // if quote���a��N�O��LP���a�� (���J��quote)

      i++ ; 
    } // while

    while ( ! indexToInsertLP.empty() ) { // �}�linsert LP(�bquote��index ) 
      int index = indexToInsertLP.top() ;
      gtempLP.lastIndexOfLine = gToken.at( index ).lastIndexOfLine ;
      gToken.insert( gToken.begin()+index, gtempLP ) ;
      count++ ;
      indexToInsertLP.pop() ;
    } // while 

    return count ;
  } // SortOutQuote()


  void ReCalc( int index, int j ) { // // �q��index�Ӷ}�l�A�C��lastIndexOfLine�Ҵ�h (j+1)
    while ( index < gToken.size() && gToken.at( index ).str != "\n" ) {
      gToken.at( index ).lastIndexOfLine = gToken.at( index ).lastIndexOfLine - j - 1 ;
      index++ ;
    } // while 
  } // ReCalc()

  void DeleteGToken( int lastIndexOfToken ) { // �R���w�ئn��gToken
    int i = 0 ;
    int lastIndexOfLine = 0 ; 
    if ( lastIndexOfToken == -1 ) ;
    else if ( lastIndexOfToken >= gToken.size() ) { // ���M��
      gToken.clear() ; 
    } // else if 
    else { // �����M��
      lastIndexOfLine = gToken.at( lastIndexOfToken ).lastIndexOfLine ;
      while ( ! gToken.empty() && i <= lastIndexOfToken ) {

        gToken.erase( gToken.begin() ) ; // �R���Ĥ@�Ӥ���A�᭱�|���ɤW�ӡA�ҥH�û��R�Ĥ@��
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
              bool error ) { // �C�L�ӪF�� �N�L�Ů� �����A���~ ��L�L����

    if ( tree == NULL ) { // �k��ONULL (���䤣�i��ONULL) 
      rp++ ;
      int times = lp - rp ;
      while ( times > 0 ) {
        cout << "  " ; // �L��ӪŮ�
        times-- ;
      } // while

      if ( parent != "OMGISCOMMENT" ) {
        cout << ")\n" ;
        parent = ")" ;
      } // if 
    } // if 
    else if ( tree->left == NULL && tree->right == NULL ) { // ���쩳�F
      int times = 0 ;
      bool hasPrintSpace = false ;   
      if ( isRightChild && tree->token.type == NIL ) { // ���쩳�B���H�ONULL
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
      else { // ���쩳 ���H�O��L�F��
        if ( parent == "(" ) { // �Y�W�@�ӬO���A�� �N���ΦL�ť�
          ;
        } // if
        else { // �W�@�Ӥ��O���A�� �ҥH�n�L�ť�      
          times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // �L��ӪŮ�
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

      if ( isRightChild ) { // �k�䨫�쩳 �N���J��k�A��
        int times = lp - 1 ;
        while ( times > 0 ) {  
          cout << "  " ;
          times-- ;
        } // while 

        cout << ")\n" ;
        parent = ")" ;
        rp++ ;
      } // if 
    } // else if ���쩳�F
    else { 
      bool dontNeedSpace = false ; 
      if ( isLeftChild ) { // �|�����쩳 �B������Node
        // if ( parent != "(" && parent != "" ) {
        if ( parent != "(" ) {
          int times = lp - rp ;
          while ( times > 0 ) {
            cout << "  " ; // �L��ӪŮ�
            times-- ;
          } // while
        } // if 

        cout << "( " ;
        lp++ ;
        parent = "(" ; 
      } // if 


      Print( tree->left, true, false, lp, rp, parent, error ) ; // ��

      if ( ( tree->str != "." && tree->str != "" && tree->str != "(" 
           )
           || tree->right == NULL
           || tree->str == "MimiNote:DontNeedToPutTwoSpace"
           || tree->right->token.type == NIL ) 
        dontNeedSpace = true ;
                           
      if (  dontNeedSpace || parent == "(" || parent == "" || tree->str == "" ) { 
      // �Y�W�@�ӬO���A�� �N���ΦL�ť�  
        ;
      } // if
      else if ( tree->str == "MimiNote:NeedToPutTwoSpace" ) {
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // �L��ӪŮ�
          times-- ;
        } // while
      } // else if 
      else { // �W�@�Ӥ��O���A�� �ҥH�n�L�ť�      
        int times = lp - rp ;
        while ( times > 0 ) {
          cout << "  " ; // �L��ӪŮ�
          times-- ;
        } // while
        
        
      } // else   

      if ( tree->str != "(" && tree->str != "" && tree->str != "MimiNote:DontNeedToPutTwoSpace" 
           && tree->str != "MimiNote:NeedToPutTwoSpace" ) // ��
        cout << tree->str << "\n" ;

      parent = tree->str ;
      Print( tree->right, false, true, lp, rp, parent, error ) ; // �k
    } // else
  } // Print()
  // ------------------------------------------------------------------------------------------- 
  
  bool IsSExp( NodePtr cur ) { // �P�_cur���V���l��O�_��S-Expression 
    if ( cur == NULL ) return true ; 
    else if ( cur != NULL && IsATOM( cur->token.type ) ) { // Atom
      return true ; 
    } // else if ATOM
    else if ( cur != NULL && cur->token.type == QUOTE ) { // QUOTE <S-exp>
      if ( IsSExp( cur->right ) == false ) { // ��index�]��ӵ������a��
        return false ; 
      } // if 
      else return true ; 
    } // else if QUOTE <S-exp>
    else if ( cur != NULL && ( cur->token.type == LEFT_PAREN || cur->token.type == DOT ) ) { 
      // ���A�� <S-exp>{<S-exp>}[DOT<S-exp>]�k�A��
      if ( IsSExp( cur->left ) == false ) { // �P�_���l�� 
        return false ;
      } // if 

      if ( IsSExp( cur->right ) == false ) { // �P�_�k�l�� 
        return false ;
      } // if

      return true ;
    } // else if ���A��


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

  // �P�_�O�_���w�q�L��Symbol
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

  // �Y��List�h�k�䨫�쩳�ONULL 
  bool IsList( NodePtr tree ) {
    NodePtr pre = tree ;
    while ( tree != NULL ) {   
      pre = tree ;  
      tree = tree->right ;
    } // while 

    if ( pre->token.type == NIL ) return true ;
    else if ( IsATOM( pre->token.type ) ) return false ; // �u��O�s���A����OATOM    
    else return true ;
  } // IsList() 

  // �P�_�O�_���w�q�L��Function
  bool IsBoundFunction( Token token ) {  
    // �Ѥj�W�w��function 
    if ( token.funcType == CONS || token.funcType == QUOTE_FN || token.funcType == DEFINE ||
         token.funcType == PART_ACCESSOR || token.funcType == PRIMITIVE_PREDICATE || 
         token.funcType == BEGIN_FN || token.funcType == OPERATOR_FN || token.funcType == EQU_TEST || 
         token.funcType == COND_FN || token.funcType == CLEAN_ENVIRONMENT || token.funcType == EXIT ||
         token.funcType == LET || token.funcType == LAMBDA ) {
      return true ;
    } // if
    else if ( token.str == "exit" ) { // ���T�w�藍�� ���o�˼g 
      return true ;
    } // else if 
    
    // ��L�۩w�q��Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( token.str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;     
  } // IsBoundFunction()

  bool IsTopLevel( NodePtr cur ) { // �P�_�O���Otop Level 
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

  bool IsPair( NodePtr cur ) { // ( ( x 5 ) ( y '(1 2 3)) ) LET���P�_ 
    while ( cur != NULL && cur->token.type != NIL ) {
      if ( cur->left != NULL && ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) ) {
        return false ;
      } // if 
      else { // ( x 5 ) �P�_�A���̭� 
        NodePtr temp = cur->left ; // temp�O���A��  
        if ( temp->left != NULL && temp->left->token.type != SYMBOL ) { // ���ӭn�OSymbol 
          return false ;
        } // if  

        temp = temp->right ;
        if ( temp == NULL ) return false ; // �S���w�q�� �h ERROR 
        if ( temp->left != NULL && IsSExp( temp->left ) == false ) { 
          // �P�_�U�@�Ӧa��O���Osexp (����SExp)
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
              if ( cur != NULL ) { // (define a 10 20 ) �u�঳�@�ӰѼ� ex: ( define a 10 ) 
                if ( cur->token.type == NIL ) {
                  return true ;
                } // if 

                errormsg = "ERROR (DEFINE format) : " ;
                return false ;
              } // if 
            
              return true ; // �᭱�ievaluation() 
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
          while ( cur != NULL && cur->token.type != NIL ) { // �P�_( F x y ) �O���O���Osymbol 
            if ( IsATOM( cur->left->token.type ) == false ) { 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 
            else if ( cur->left->token.type != SYMBOL ) { // �Dsymbol ( F 3 ) 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // else if

            if ( first && IsProcedureFn( cur->left->token ) ) {  // ���i�H�w�q�Ѥj�w�q��function 
              errormsg = "ERROR (DEFINE format) : " ; 
              return false ;
            } // if 

            cur = cur->right ;
          } // while 

          bone = bone->right ;
          
          while ( cur != NULL && cur->token.type != NIL ) { // 1~�h�� S-Exp 
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
        cur = cur->right ; // // ���U�@���s��(�Ѽ�)�ˬd�䥪�l��O�_�O<S-exp>     
      } // while 
      
      if ( ( cur == NULL || cur->token.type == NIL ) && isSExp && isDoubleTon ) { // ��k���T 
        return true ;
      } // if 
      else { // ��k���~ 
        errormsg = "ERROR (COND format) : " ;
        return false ;
      } // else 
    } // else if
    else if ( type == LET ) { // LET
      int numOfArgs = CountNumOfArgs( cur ) ; // cur �q�Ĥ@�ӰѼƪ��s���}�l�� 
      if ( numOfArgs != 2 && numOfArgs < 2 ) { // �ѼƳ����u������s�� (����let������) 
        errormsg = "ERROR (LET format) : " ;
        return false ;
      } // if 
      else {
        if ( IsATOM( cur->left->token.type ) && cur->left->token.type == NIL ) ;
        else if ( IsATOM( cur->left->token.type ) || cur->left->token.type == QUOTE ) { 
          // ���ӭn�O�@��node ����O��ATOM (���ӭn�O���A��) 
          errormsg = "ERROR (LET format) : " ;
          return false ;
        } // else if
        else { // �Ĥ@�ӰѼƬO���A���}�Y (zero-or-more-PAIRs)  ( ( x 5 ) ( y '(1 2 3))) 
          if ( IsPair( cur->left ) == false ) { // �ǳo�Ӷi�h�P�_ ( ( x 5 ) ( y '(1 2 3)) )
            errormsg = "ERROR (LET format) : " ;
            return false ;
          } // if 
        } // else 
      } // else 
    } // else if LET 
    

    return true ;
  } // CheckFormat()

  int CountNumOfArgs( NodePtr cur ) { // ��argument���ƶq 
    if ( cur != NULL && cur->left != NULL ) {
      return CountNumOfArgs( cur->right ) + 1 ;
    } // if
    else {
      return 0 ;
    } // else 
  } // CountNumOfArgs()

  void CheckNumOfArgs( FunctionType type, NodePtr in, int numOfArgs, bool &error, string & errormsg ) {
    // lambda��errormsg�i��n�t�~�B�z (�b��FN���B�z) 
    if ( type == CONS ) {
      if ( in->left->str == "cons" && numOfArgs != 2 ) { // cons(2)
        error = true ;  
      } // if �ѼƼƶq���~
      else if ( in->left->str == "list" && numOfArgs < 0 ) { // list(>=0) 
        error = true ;  
      } // else if 
      else {
        error = false ;
      } // else 
    } // if
    else if ( type == QUOTE_FN ) { // quote(1)
      if ( numOfArgs != 1 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == PART_ACCESSOR ) { // car(1), cdr(1)
      if ( numOfArgs != 1 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if 
    else if ( type == PRIMITIVE_PREDICATE ) { // ���u��1�� �p: atom? pair? null? integer?... 
      if ( numOfArgs != 1 ) { // �ѼƼƶq���~
        error = true ;   
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == OPERATOR_FN ) { // +-*/ > = < not and or...
      if ( in->left->str == "not" && numOfArgs != 1 ) { // not(1)
        error = true ;           
      } // if �ѼƼƶq���~
      else if ( in->left->str != "not" && numOfArgs < 2 ) { // ��L���n >= 2 
        error = true ;
      } // else if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EQU_TEST ) { //   eqv?(2) equal?(2)
      if ( numOfArgs != 2 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == BEGIN_FN ) { // begin
      if ( numOfArgs < 1 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "cond" && numOfArgs < 1 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == COND_FN ) { // begin
      if ( in->left->str == "if" && ( numOfArgs != 2 || numOfArgs != 3 ) ) { // �ѼƼƶq���~
        error = true ;   
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CLEAN_ENVIRONMENT ) { // clear_env.
      if ( numOfArgs != 0 ) { // �ѼƼƶq���~
        error = true ;  
      } // if �ѼƼƶq���~
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == CUSTOMIZE ) { // �۩w�q��FN 
      Fn f ;
      int i = 0 ;
      bool found = false ;
      while ( i < gFunctionTAB.size() && !found ) { // �qgFunctionTAB���䦹FN 
        if ( in->left->str == gFunctionTAB.at( i ).str ) { // ��즹FN 
          f = gFunctionTAB.at( i ) ;
          found = true ;
        } // if
        
        i++ ; 
      } // while
      
      if ( numOfArgs != f.numOfArgs ) { // �P�_�ѼƼƶq�藍�� 
        error = true ;        
      } // if 
      else {
        error = false ;
      } // else 
    } // else if
    else if ( type == EXIT ) {
      if ( numOfArgs != 0 ) { // �P�_�ѼƼƶq�藍�� 
        error = true ;        
      } // if
      else error = false ; 
    } // else if 

    if ( error ) {
      errormsg = "ERROR (incorrect number of arguments) : " + in->left->str + "\n" ; 
    } // if error
  } // CheckNumOfArgs()

  bool DoNotNeedEvalARGS( NodePtr in ) { // DEFINE/ QUOTE/ COND/ if/ and/ or ���ݭn�P�_�U�@��args �ҥH�n���L

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
    if ( useLocal ) { // ��symbol�i��Olocal  
      while ( i < glocal.size() ) {
        if ( glocal.at( i ).str == node->token.str ) {
          return glocal.at( i ).value ;
        } // if 
      
        i++ ;
      } // while 
    } // if 

    // �Ylocal�S��� 
    i = 0 ;
    while ( i < gSymbolTAB.size() ) {
      if ( gSymbolTAB.at( i ).str == node->token.str ) {
        return gSymbolTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 

    // SYMBOL �S��� ���function 
    i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( gFunctionTAB.at( i ).str == node->token.str ) {
        return gFunctionTAB.at( i ).value ;
      } // if 
      
      i++ ;
    } // while 
        
    return node ;
  } // GetDefineSymbolNode()   
  
  bool IsCustomize( string str ) { // �O�۩w�q��function 
    // ��L�۩w�q��Function 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( str == gFunctionTAB.at( i ).str ) {
        return true ;
      } // if 
      
      i++ ;
    } // while
    
    return false ;
  } // IsCustomize() 

  bool CheckCustomizeArgs( string str, NodePtr in ) { // �P�_�۩w�q��fn���ѼƬO�_�ۦP 
     // ��L�۩w�q��Function 
    int i = 0 ;
    int numOfArgs = 0 ; 
    int index = 0 ; // �Ӧ۩w�q��function�b���Ӧ�m 
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) {
      if ( str == gFunctionTAB.at( i ).str ) { // ���F 
        toBreak = true ;
        index = i ;
      } // if 
      
      i++ ;
    } // while

    i = 0 ;
    while ( in != NULL && in->token.type != NIL ) { // �p��(�ؾ��)���h�ְѼ� 
      numOfArgs++ ;
      in = in->right ;
    } // while 

    if ( numOfArgs == gFunctionTAB.at( index ).numOfArgs ) return true ;
    else return false ;  
  } // CheckCustomizeArgs()

  void DefineLocal( string funcName, NodePtr in, bool useLocal, bool & error, string & errormsg, 
                    NodePtr & errorNode ) { 
    // �w�qlocal variable �hfunction�~�వ�� 
    // (��function�W�A�Ѽ�(�_�l)�Ҧb��bone) 
    int i = 0 ;
    bool toBreak = false ;
    bool find = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // �����function�A���n�w�q���Ѽƭ� 
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
    NodePtr in_temp = in ; // ����in�@�}�l����m 
    // �N�Ҧ��n�Q�w�q���ȥ��p�⧹�� 
    while ( in != NULL && in->token.type != NIL && error == false ) { 
      Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        errormsg = "ERROR (unbound symbol) : " ;
        errorNode = in->left ;
      } // if 

      in->left = out ; // �Ѽƪ��� 
      in = in->right ; // ���U�@�ӰѼ� 
    } // while 

    // �@�@�N�n�Q�w�q���Ѽƨ��X�áA������ 
    i = 0 ;
    in = in_temp ; 
    while ( error == false && i < f.args.size() ) { // �N�ѼƤ@�@���X�A�éw�q 
      s.str = f.args.at( i ) ; // �ѼƦW��
      s.value = in->left ; // �Ѽƪ���
      if ( s.value != NULL && IsBoundSymbol( s.value->token ) ) {
        DeleteDefineSym( s.str ) ;
      } // if 

      glocal.push_back( s ) ; // �s�Jglocal�� 
      in = in->right ; // ���U�@�ӰѼ� 
      i++ ;
    } // while 

  } // DefineLocal()

  NodePtr GetFnDefine( string fnName ) { // ���ofnName�o��function�n���檺�� 
    int i = 0 ;
    bool toBreak = false ;
    while ( i < gFunctionTAB.size() && toBreak == false ) { // �����function�A���n�w�q���Ѽƭ� 
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

  bool IsProcedureFn_Str( string str, FunctionType & fnType ) { // �Y��string�O�Ѥj�w�q��finction �^��type 
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

  bool CheckLambdaFormat( NodePtr in ) { // �ˬdlambda��format�O�_�ŦX 
    // (lambda (zero-or-more-symbols�����Oconstant) one-or-more-S-expressions )

    NodePtr bone = in->right ; // �ѼƩҦbbone 
    if ( bone == NULL || bone->token.type == NIL ) return false ; // ( lambda ) 
    in = in->right ;
    // ���ˬd (zero-or-more-symbols�����Oconstant)
    if ( IsATOM( in->left->token.type ) && in->left->token.type == NIL ) ; // () �s��symbol
    else if ( IsATOM( in->left->token.type ) ) return false ; // ( lambda 5 )
    else { // �Ĥ@�ӰѼ� 1~�h��symbol()�A�����O�Ʀr�������u�Osymbol 
      in = in->left ;
      while ( in != NULL && in->token.type != NIL ) {
        if ( IsATOM( in->left->token.type ) == false ) return false ;
        else if ( in->left->token.type != SYMBOL ) return false ;
        in = in->right ; 
      } // while 
    } // else 

    // �A�ˬd one-or-more-S-expressions
    bone = bone->right ; // �ĤG�ӰѼƩҦbbone   
    if ( bone == NULL || bone->token.type == NIL ) return false ; // �@�w�n1~�h�� S-Exp 
    while ( bone != NULL && bone->token.type != NIL ) {
      in = bone->left ; // �ĤG�ӰѼƩҦb��m
      if ( IsSExp( in ) == false ) return false ;
      bone = bone->right ;
    } // while 

    return true ;
  } // CheckLambdaFormat()

  void CheckArgsType( NodePtr in, NodePtr para, bool & error, string & errormsg ) { // in = (root)
    // check para ��type�O�_����in->str �o��function 
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

  } // CheckArgsType() // ( fn�W, �Ѽ�)	

  void GetBoundSymbol( NodePtr in, NodePtr & out, bool useLocal ) {
    bool find = false ;
    if ( IsBoundFunction( in->token ) ) { // �O�Q�w�q�L��function 
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
    } // if �O�Q�w�q�L��function
    else {
      out = in ; 
      int i = 0 ;
      if ( useLocal ) { // �d��ϰ��ܼ�
                    
        while ( i < glocal.size() ) {
          if ( in->token.str == glocal.at( i ).str ) { 
            out = glocal.at( i ).value ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // �Olambda�B��top level �h�L<procedure lambda > 
            } // if 

            find = true ;
          } // if 
      
          i++ ;
        } // while

      } // if 

      i = 0 ;
      if ( find == false ) { // �d������ܼ� 
        while ( i < gSymbolTAB.size() && find == false ) {
          if ( in->token.str == gSymbolTAB.at( i ).str ) { 
            out = gSymbolTAB.at( i ).value ;
            FunctionType fnType = NONE ;
            if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
              out = out->left ; // �Olambda�B��top level �h�L<procedure lambda > 
            } // if 
            else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
              // ���F�Ѥj�w�q��function�� "�Qquote"�H�~�� top level function (�۩w�qfunction)
              out->token.funcType = fnType ;
            } // else if 

            find = true ; 
          } // if 
      
          i++ ;
        } // while
      } // if  ������ܼ�    
    } // else

    if ( find == false ) { // �S�w�q��symbol 
      out = in ;
    } // if
  } // GetBoundSymbol()
  
  void Eval( NodePtr in, NodePtr & out, bool & error, string & errormsg, NodePtr &errorNode, 
             bool useLocal ) { // useLocal = �O�_�ϥ�local variable 
    bool done = false ; // define/ cond/ if/ and / or���������Nreturn���G 
    bool customizeDone = false ;
    vector<Symbol> glocalTemp ; // �s�ª�glocal�Afn��������A�s�^�h
    NodePtr copy_tree = NULL ;
    CopyTree( copy_tree, in ); 
    
    if ( IsATOM( in->token.type ) && in->token.type != SYMBOL ) { // ATOM 
      out = in ; // return that atom
      out->left = NULL ;
      out->right = NULL ;
    } // if ATOM
    else if ( in->token.type == SYMBOL ) { // SYMBOL 
      bool find = false ;
      if ( IsBoundFunction( in->token ) ) { // �O�Q�w�q�L��function 
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
      } // if �O�Q�w�q�L��function
      else {
        out = in ; 
        int i = 0 ;
        if ( useLocal ) { // �d��ϰ��ܼ�
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // �Olambda�B��top level �h�L<procedure lambda > 
              } // if 

              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if 

        i = 0 ;
        if ( find == false ) { // �d������ܼ� 
          while ( i < gSymbolTAB.size() && find == false ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
              FunctionType fnType = NONE ;
              if ( gTree == in && out->left != NULL && out->left->token.funcType == LAMBDA ) {
                out = out->left ; // �Olambda�B��top level �h�L<procedure lambda > 
              } // if 
              else if ( IsProcedureFn_Str( out->str, fnType ) && out->token.funcType != BEEN_QUOTE ) { 
                // ���F�Ѥj�w�q��function�� "�Qquote"�H�~�� top level function (�۩w�qfunction)
                out->token.funcType = fnType ;
              } // else if 

              find = true ; 
            } // if 
      
            i++ ;
          } // while
        } // if  ������ܼ�    
      } // else

      if ( find == false ) { // �S�w�q��symbol 
        out = in ;
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->str + "\n" ;
      } // if
    } // else if SYMBOL
    else { // �����@��node ( root )
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
        if ( IsBoundFunction( in->left->token ) ) { // �O�w�q�L��Function // 
           
          // ���OTopLevel �B��define clear env.------------------------------------------(A-START)
          // PART_A
          bool isTopLevel = IsTopLevel( in->left ) ;
         
          if ( isTopLevel == false  &&  in->left->token.funcType == DEFINE ) { // (A-1) 
            error = true ;
            errormsg = "ERROR (level of DEFINE)\n" ;        
          } // if ���OTopLevel �B��define
          else if ( isTopLevel == false  && in->left->token.funcType == CLEAN_ENVIRONMENT ) {
            error = true ;
            errormsg = "ERROR (level of CLEAN-ENVIRONMENT)\n" ;
          } // else if ���OTopLevel �B��clear_env. 
          else if ( isTopLevel == false  && in->left->token.str == "exit" ) {
            error = true ;
            errormsg = "ERROR (level of EXIT)\n" ;
          } // else if ���OTopLevel �B��exit.  -----------------------------------------------------(A-1)
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
              errorNode = in ; // error��������Node �p: (define ) �^�ǫ��V�̥~�����A��() 
              error = true ;
            } // if
            else { // format���T�A�p����tree(�qin�}�l)
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
            if ( IsCustomize( in->left->str ) ) { // �۩w�q��function 
              if ( CheckCustomizeArgs( in->left->str, in->right ) == false ) {
                error = true ;
                errormsg = "ERROR (incorrect number of arguments) : " ;
                bool temp_error = false ;
                Eval( in->left, errorNode, temp_error, errormsg, errorNode, useLocal ) ; 
              } // if 
              else { // �ѼƼƶq���T 
                in->left->token.funcType = CUSTOMIZE ;
                glocalTemp = glocal ;
                // ��function�W�A�Ѽ�(�_�l)�Ҧb��bone
                DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
                NodePtr fnDefine = GetFnDefine( in->left->str ) ; // ���o��function�n������ 
                useLocal = true ;
                
                // eval( Second arg S2 of the main S-exp)
                NodePtr in_args = fnDefine ; // in_args���䦳�Ѽƪ��s��  in ��main_S-exp 
                NodePtr out_result = NULL ;
                bool paraCorrect = true ; 
                if ( error ) { // �ѼƬO�_���� 
                   paraCorrect = false ;
                } // if 

                while ( in_args != NULL && in_args->token.type != NIL && paraCorrect ) { 
                  // ��node��left���Ѽ�
	              if ( error && errormsg == "ERROR (unbound parameter) : " ) {
                    error = false ;
                    errormsg = "" ;	
	              } // if 

                  Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = �Ѽ�
                  in_args->left = out_result ; // ���_��
                  if ( error == false ) {
                    CheckArgsType( in, out_result, error, errormsg ) ; // ( fn�W(token), �Ѽ�)	
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

                  in_args = in_args->right ; // ���U�@���s�� 
                } // while

                out = out_result ; 
                if ( paraCorrect && out == NULL ) {
                  error = true ;
                  errormsg = "ERROR (no return value) : " ;
                  errorNode = copy_tree ;
                } // if 
                /*
                if ( error == false ) { // �Lerror�~�~�򰵤U�h 
                  Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // ���榹function
                  if ( error && errormsg == "ERROR (no return value) : " ) errorNode = copy_tree ;
                } // if 
                */               
                customizeDone = true ;
              } // else 
            } // if 
            else { // �D�۩w�qfunction�p: car cdr 
              int numOfArgs = CountNumOfArgs( in->right ) ;
              // Check numof args �O�_���T 
              CheckNumOfArgs( in->left->token.funcType, in, numOfArgs, error, errormsg ) ;
            } // else 
                          
          } // else --------------------------------------------------------------------------------(A-4)
          
          // ---------------------------------------------------------------------------------------(A-END)
        } // if �O�w�q�L��Function 
        else { // ��SYMBOL���O�Q�w�q�L��function�W�� 
          if ( IsBoundSymbol( in->left->token ) == false ) { // SYMBOL�|���Q�w�q
            error = true ; 
            errormsg = "ERROR (unbound symbol) : " + in->left->str + "\n" ;
          } // if 
          else { // �w�w�q��SYMBOL�����Ofunction
             
            NodePtr define_node = GetDefineSymbolNode( in->left, useLocal ) ;
            NodePtr temp_node = NULL ;
            CopyTree( temp_node, define_node ) ; 
            in->left = temp_node ;
            if ( IsProcedureFn( define_node->token ) ) { // �O�w�q�L��"�Ѥj���wFN" �H���_�ӤF �ҥH���ΰ��� 
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
        } // else  ��SYMBOL���O�Q�w�q�L��function�W�� 
      } // else if first argument is SYMBOL
      else { // ( (�C�C�C) ... ) �� (�C�C�C)
        // evaluate (�C�C�C) �]�N�Oin->left(0) 
        NodePtr temp = in ;
        NodePtr tempOut = NULL ;
        if ( in->left != NULL && in->left->left != NULL && 
             in->left->left->token.funcType == LAMBDA ) { // �Y��lambda 
          useLocal = true ;
        } // if
        else { // �Dlambda����L 
          Eval( in->left, tempOut, error, errormsg, errorNode, useLocal ) ;
        
          if ( error == false ) {
            in->left = tempOut ; // ���_�� 
            // ----------------------------------------------------------------------- PART_B (START) 
            // ����Fn�W��  �Ytemp�����OATOM �N���OFN  
            if ( IsATOM( tempOut->token.type ) && IsBoundFunction( tempOut->token ) ) { 
              // ( IsATOM( tempOut->token.type ) == false && IsBoundFunction( tempOut->left->token ) ) 
              int numOfArgs = CountNumOfArgs( temp->right ) ; // in->right����FN���Ѽ��s��1 
              // Check numof args �O�_���T 
              CheckNumOfArgs( temp->left->token.funcType, temp, numOfArgs, error, errormsg ) ;
              // lambda��errormsg�i��n�t�~�B�z �άOCheckNumOfArgs���B�z 
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
            else { // �DFn�W�� 
              error = true ;
              errormsg = "ERROR (attempt to apply non-function) : " ;
              // errormsg = errormsg + tempOut->str + "\n" ;
              errorNode = tempOut ;  
            } // else
          
            // ---------------------------------------------------------------------------(PART_B) END 
          } // if
        } // else �Dlambda����L
          
        
      } // else ( (�C�C�C) ... ) �� (�C�C�C)
      
      
      NodePtr out_result = NULL ;
      if ( DoNotNeedEvalARGS( in->left ) || in->left->token.funcType == LAMBDA ) { 
        // ���ǫ��O���ݭn�P�_�U�@�ӰѼơA�n�����i��B�� 
        ; 
      } // if
      else if ( in->left != NULL && in->left->left != NULL && 
                in->left->left->token.funcType == LAMBDA ) {
        useLocal = true ;
      } // else if 
      else { // ��QUOTE���n�~��P�_�U�@�ӰѼ� 
        // eval( Second arg S2 of the main S-exp)
        NodePtr in_args = in->right ; // in_args���䦳�Ѽƪ��s��  in ��main_S-exp 
        while ( in_args != NULL && in_args->token.type != NIL ) { // ��node��left���Ѽ�
	      if ( error && errormsg == "ERROR (unbound parameter) : " ) {
            error = false ;
            errormsg = "" ;	
	      } // if 

          Eval( in_args->left, out_result, error, errormsg, errorNode, useLocal ) ; // in_args->left = �Ѽ�
          in_args->left = out_result ; // ���_��
          if ( error == false ) {
            CheckArgsType( in, out_result, error, errormsg ) ; // ( fn�W(token), �Ѽ�)	
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

          in_args = in_args->right ; // ���U�@���s�� 
        } // while
      } // else 
        

      if ( error == false && customizeDone == false ) { 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // �p�⵲�G�æ^��
        out = out_result ;
      } // if
      else if ( error && errormsg == "ERROR (unbound parameter) : " ) {
        error = false ;
        errormsg = "" ; 
        Evaluate( in, out_result, error, errormsg, errorNode, useLocal, glocalTemp, copy_tree ) ; // �p�⵲�G�æ^��
        out = out_result ; 
      } // else if 
      else { // �۩w�q�����槹 
        glocal = glocalTemp ; // �N��glocal�s�^�� �]�N�O�Nlocal variables pop��
      } // else 

      // ---------------------------------------------------------------------------(PART_C) END  
    } // else 
  } // Eval()

  bool IsPair( NodePtr cur, string & errormsg ) { // (cur = root)

    NodePtr fn = cur->left ; // fn �Ȧs(���V) car, cdr�o��function��Node (errormsg�ݭn�L�X��fn�W�ɥ�)
    cur = cur->right ; // (1)
    if ( cur->right != NULL && cur->right->token.type != NIL ) { // ex: (car 3 4) error (�]���u�঳�@�ӰѼ�) 
      errormsg = "ERROR (incorrect number of arguments) : " + fn->str + "\n" ;        
      return false ;
    } // if  ex: (car 3 4) error
    
    // Token define_token = GetDefineToken( cur->left->token ) ; 
    if ( IsATOM( cur->left->token.type ) ) { // ( car 3 ) cur->left �N�O 3 �o��node 
      errormsg = "ERROR (" + fn->str + " with incorrect argument type) : " ;
      return false ;
    } // if ( car 3 ) cur->left �N�O 3 �o��node 
    
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

  void BeenQuoted( NodePtr cur ) { // �Ncur�ҫ��V���𪺰Ѽ� �����S��N�q(�Qquote�_�Ӫ��N��)
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

  void FloatStrToIntStr( string in, string & out ) { // �Nfloat�୼int(���ҥHstring���覡�x�s) 
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
    // �Ncur�ҫ��V���𪺩Ҧ�"�Ѽ�"�ۥ[ 
    float sum = 0 ;
    float num = 0 ; // �Ψӱ��Ѽƪ�float(�쬰string)
    bool isInt = true ; // �����Ofloat���[�k�٬Ointeger���[�k 
    result = new Node() ; // �^�Ƿs��Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // �নfloat�A�B��
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type���Oint�]���Ofloat 
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
    // �Y�O3.0 �B�n�Lint �N�|�����L�X3
    // �Y�O�n�Lfloat �L"%.3f"�]�|�O�諸����
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
    // �Ncur�ҫ��V���𪺩Ҧ�"�Ѽ�"�ۥ[ 
    bool isInt = true ; // �����Ofloat���[�k�٬Ointeger���[�k 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼ�type�����T 
      error = true ;
      errormsg = "ERROR (- with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // �Ĥ@�ӰѼ�type���T 
      float sum = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼ� 
      if ( cur->left->token.type == FLOAT ) { // �P�_�Ĥ@�ӰѼƪ�type 
        isInt = false ;
      } // if 
      
      float num = 0 ; // �Ψӱ��Ѽƪ�float(�쬰string)    
      result = new Node() ; // �^�Ƿs��Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // ���U�@�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        num = atof( cur->left->str.c_str() ) ; // �নfloat�A�B��
        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type���Oint�]���Ofloat 
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
      // �Y�O3.0 �B�n�Lint �N�|�����L�X3
      // �Y�O�n�Lfloat �L"%.3f"�]�|�O�諸���� 
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
    // �Ncur�ҫ��V���𪺩Ҧ�"�Ѽ�"�ۥ[ 
    float sum = 1 ;
    float num = 0 ; // �Ψӱ��Ѽƪ�float(�쬰string)
    bool isInt = true ; // �����Ofloat���[�k�٬Ointeger���[�k 
    result = new Node() ; // �^�Ƿs��Node 
    result->left = NULL ;
    result->right = NULL ;
    result->token.funcType = NONE ;
    error = false ;
    while ( cur != NULL && cur->token.type != NIL && error == false ) {
      num = atof( cur->left->str.c_str() ) ; // �নfloat�A�B��
      if ( cur->left->token.type == FLOAT ) {
        isInt = false ;
      } // if
      
      if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type���Oint�]���Ofloat 
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
    // �Y�O3.0 �B�n�Lint �N�|�����L�X3
    // �Y�O�n�Lfloat �L"%.3f"�]�|�O�諸���� 
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
    // �Ncur�ҫ��V���𪺩Ҧ�"�Ѽ�"�ۥ[ 
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼ�type�����T 
      error = true ;
      errormsg = "ERROR (/ with incorrect argument type) : " ;
      errorNode = cur->left ;
    } // if
    else { // �Ĥ@�ӰѼ�type���T 
      bool isInt = true ; // �����Ofloat���[�k�٬Ointeger���[�k
      // float sum = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼ� 
      double sum ;
      stringstream ss ;
      ss << cur->left->str ;
      ss >> sum ;
      
      
      
      // double sum = atof( cur->left->str.c_str() ) ;
      if ( cur->left->token.type == FLOAT ) { // �P�_�Ĥ@�ӰѼƪ�type 
        isInt = false ;
      } // if 
      
      // float num = 0 ; // �Ψӱ��Ѽƪ�float(�쬰string) 
      double num = 0 ;
      result = new Node() ; // �^�Ƿs��Node 
      result->left = NULL ;
      result->right = NULL ;
      result->token.funcType = NONE ;
      error = false ;
      cur = cur->right ; // ���U�@�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && error == false ) {
        // num = atof( cur->left->str.c_str() ) ; // �নfloat�A�B��

        stringstream ss ;
        ss << cur->left->str ;
        ss >> num ;



        if ( cur->left->token.type == FLOAT ) {
          isInt = false ;
        } // if
        
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // type���Oint�]���Ofloat 
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
      // �Y�O3.0 �B�n�Lint �N�|�����L�X3
      // �Y�O�n�Lfloat �L"%.3f"�]�|�O�諸���� 
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
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // �s�����H���ONIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ; // ���X�j�� 
      } // if 
      else if ( temp->token.type == NIL ) { // ���Ĥ@��NIL �N�^��NIL 
        result = temp ;
        toBreak = true ; // ���X�j�� 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // �N���ӪF�賣�S��NIL 
      result = lastSExp ; // �h�^�ǳ̫�@��S-Exp 
    } // if 
  } // And()

  void Or( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal ) { // Or
    NodePtr lastSExp = NULL ;
    bool toBreak = false ;
    NodePtr temp = NULL ;
    while ( cur != NULL && cur->token.type != NIL && toBreak == false ) { // �s�����H���ONIL 
      Eval( cur->left, temp, error, errormsg, errorNode, useLocal ) ;
      if ( error ) {
        toBreak = true ;
      } // if 
      else if ( temp->token.type != NIL ) {
        result = temp ;
        toBreak = true ; // ���X�j�� 
      } // else if 
      else {
        lastSExp = temp ;
        cur = cur->right ;
      } // else 
    } // while 
    
    if ( toBreak == false ) { // �N���ӪF�賣�ONIL 
      result = lastSExp ; // �h�^�ǳ̫�@��S-Exp (�]�N�ONIL) 
    } // if 
  } // Or()

  void BiggerThan( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr t, NodePtr f, 
                   NodePtr & errorNode ) {
    bool toBreak = false ; 
    bool isTrue = true ;
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (> with incorrect argument type) : " ; 
      errorNode = cur->left ;     
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼƬO�Ʀr 
      float f2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (> with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          f2 = atof( cur->left->str.c_str() ) ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (>= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼƬO�Ʀr 
      float f2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (>= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          f2 = atof( cur->left->str.c_str() ) ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (< with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼƬO�Ʀr 
      float f2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (< with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          f2 = atof( cur->left->str.c_str() ) ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (<= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼƬO�Ʀr 
      float f2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (<= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          f2 = atof( cur->left->str.c_str() ) ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (= with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      float f1 = atof( cur->left->str.c_str() ) ; // �Ĥ@�ӰѼƬO�Ʀr 
      float f2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != INT && cur->left->token.type != FLOAT ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (= with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          f2 = atof( cur->left->str.c_str() ) ; // ���ѼƬO�Ʀr 
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
    
    if ( cur->left->token.type != STRING ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string>? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // �Ĥ@�ӰѼƬO�Ʀr 
      string s2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string>? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          s2 = cur->left->str ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != STRING ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string<? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // �Ĥ@�ӰѼƬO�Ʀr 
      string s2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string<? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          s2 = cur->left->str ; // ���ѼƬO�Ʀr 
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
    if ( cur->left->token.type != STRING ) { // �Ĥ@�ӰѼƤ��O�Ʀr 
      error = true ;
      result = NULL ;
      errormsg = "ERROR (string=? with incorrect argument type) : " ;      
      errorNode = cur->left ;
    } // if
    else {
      string s1 = cur->left->str ; // �Ĥ@�ӰѼƬO�Ʀr 
      string s2 ;
      cur = cur->right ; // �ĤG�ӰѼ� 
      while ( cur != NULL && cur->token.type != NIL && toBreak == false ) {  
        if ( cur->left->token.type != STRING ) { // �ĤG�ӰѼƤ��O�Ʀr 
          error = true ;
          result = NULL ;
          errormsg = "ERROR (string=? with incorrect argument type) : " ;
          errorNode = cur->left ;
          toBreak = true ;      
        } // if
        else { // ���ѼƬO�Ʀr
          s2 = cur->left->str ; // ���ѼƬO�Ʀr 
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
    string temp = "" ; // �ݥ[�J��string 
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
    NodePtr para1 = cur->left ; // �Ĥ@�ӰѼ�
    cur = cur->right ;
    bool isTrue = false ;
    NodePtr para2 = cur->left ; // �ĤG�ӰѼ� 
    if ( para1 == para2 ) {
      isTrue = true ;
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type ) ) { // ���OATOM 
      if ( para1->str == para2->str ) { // �B�ۦP
        if ( para1->token.type == STRING && para2->token.type == STRING ) { // �Y�OString�h���ҥ~ -> false 
          result = f ;  
        } // if 
        else {
          isTrue = true ;
        } // else 
      } // if 
      else { // ���OATOM���W�r(str)���P 
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
      else { // �䤤�@�ʾ𵲧��F �t�@�ʩ|������ 
        return f ;
      } // else 
    } // if
    else if ( IsATOM( para1->token.type ) && IsATOM( para2->token.type )  ) { // ���OATOM 
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
    else if ( IsATOM( para1->token.type ) == false && IsATOM( para2->token.type ) == false ) { // �����OATOM
      NodePtr status = Equal( para1->left, para2->left, t, f ) ;
      if ( status->str == "#t" ) {
        return Equal( para1->right, para2->right, t, f ) ;
      } // if 
      else {
        return f ;
      } // else  
    } // else if  �����OATOM
    else { // �䤤�@�ӬOATOM �t�@�Ӥ��O 
      return f ;
    } // else �䤤�@�ӬOATOM �t�@�Ӥ��O

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

      
      CopyTree( temp->left, cur->left ) ; // ������ 
      CopyTree( temp->right, cur->right ) ; // ���k�� 
    } // else
    
  } // CopyTree()

  void If( NodePtr cur, NodePtr & result, bool & error, string & errormsg, NodePtr &errorNode,
           bool useLocal, NodePtr copy_tree ) {
    NodePtr temp = NULL ;
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // ��cur�o�Ӿ�copy�@����temp�� 
    NodePtr tempOut = NULL ;
    temp = temp->right ; // �Ĥ@�ӰѼ� (�p�⧹����False(nil) �� �DFalse)
    Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
    // (�p�⧹����False(nil) �� �DFalse)
    if ( error == false ) {
      temp = temp->right ;
      if ( tempOut->token.type != NIL ) { // true
        Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; 
        if ( error == false ) {
          result = tempOut ;
        } // if no error
      } // if 
      else { // false
        temp = temp->right ; // �ĤT�ӰѼƩҦb�s�� 
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
    NodePtr bone = NULL ; // �t�d���s�������� 
    bool isLeftChild = true ;
    CopyTree( temp, cur ) ; // ��cur�o�Ӿ�copy�@����temp��  
    NodePtr tempOut = NULL ;
    temp = temp->right ; // �Ĥ@�ӰѼ� (�Ĥ@��if������)
    bone = temp ; // �ĤG���s��
    bool done = false ;
    while ( error == false && done == false && bone != NULL && bone->token.type != NIL ) {
      temp = bone->left ; // if/ else if / else ����m
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // (�p�⧹����False(nil) �� �DFalse)
      
      if ( error && ( bone->right == NULL || bone->right->token.type == NIL ) 
           && tempOut != NULL && tempOut->str == "else" ) {
        // �̫�@���s�� �Oelse   ( ��Lelse �Q�P�w�� unbound symbol ) 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          error = false ; // �]�^false �~���~���
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 
 
          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // �n�^�Ǫ����G 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error        

          temp = temp->right ;
        } // while 
      } // if �̫�@���s��
      else if ( error == false && ( bone->right == NULL || bone->right->token.type == NIL ) 
                && temp->left->str == "else" ) { 
        // else �Q�w�q�L ����else �O�̫�@��Sexp 
        temp = temp->right ;
        while ( temp != NULL && temp->token.type != NIL ) {
          if ( error == true && errormsg == "ERROR (no return value) : " ) {
            error = false ;
            errormsg = "" ;
          } // if 

          Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // �n�^�Ǫ����G 
          if ( error == false ) {
            result = tempOut ;
            done = true ;
          } // if no error

          temp = temp->right ;
        } // while 
      } // else if else �Q�w�q�L ����else �O�̫�@��Sexp  
      
    
      if ( done == false && error == false ) {

        if ( tempOut->token.type != NIL ) { // true
          temp = temp->right ;
          while ( temp != NULL && temp->token.type != NIL && done == false ) {
            if ( error == true && errormsg == "ERROR (no return value) : " ) {
              error = false ;
              errormsg = "" ;
            } // if 

            Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // �n�^�Ǫ����G 
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

  void DefineSym( NodePtr bone ) { // �w�qlocal variable 
    NodePtr temp = bone->left ;
    NodePtr tempOut = NULL ;
    bool error = false ; 
    string errormsg = "" ;
    NodePtr errorNode = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr temp_bone = bone ; // �Ȧs��l��bone 
    while ( bone != NULL && bone->token.type != NIL ) { // bone�O�̤j���خ� ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp�O�̭����p�A�� ( x 5 ) ��( y 3 ) 
      temp = temp->right ;
      Eval( temp->left, tempOut, error, errormsg, errorNode, useLocal ) ; // �p���symbol��value
      temp->left = tempOut ;
      bone = bone->right ; // ���U�@�ӰѼ� 
    } // while 
 
    bone = temp_bone ;
    while ( error == false && bone != NULL && bone->token.type != NIL ) { 
      // bone�O�̤j���خ� ( (x 5) ( y 3 ) ) 
      temp = bone->left ;  // temp�O�̭����p�A�� ( x 5 ) ��( y 3 ) 
      s.str = temp->left->str ; // symbol 
      temp = temp->right ;
      s.value = temp->left ;
      glocal.push_back( s ) ; // �Nlocal variable�s�Jvector�� 
      bone = bone->right ; // ���U�@�ӰѼ� 
    } // while 
  } // DefineSym()


  void DeleteDefineFn( string str ) { // �R�h�w�w�q��function 
    // ��L�۩w�q��Function 
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

  void DeleteDefineSym( string str ) { // �R�h�w�w�q��symbol
    // ��L�۩w�q��Function 
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


  int GetFnArgsNum( string fnName, vector<string> & args ) { // ���fnName�o��function��A�^�Ǩ�ѼƸ�T 
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
 
  string GetOriginFn( string fnName ) { // ���fnName�쥻�O�Q�֩w�q�� 
    int i = 0 ;
    while ( i < gFunctionTAB.size() ) {
      if ( fnName == gFunctionTAB.at( i ).str ) {
        if ( gFunctionTAB.at( i ).originFn == "" ) { // �O���function
          return fnName ;
        } // if 
        else { // �O�ѧO�H�w�q�� 
          return gFunctionTAB.at( i ).originFn ; 
        } // else 
      } // if 

      i++ ;
    } // while

    return fnName ; 
  } // GetOriginFn()

  int GetProcedureFnArgs( NodePtr f ) { // ���Ѥj�ҩw�q��function�A�æ^�ǳW�w���ѼƼƶq 
    if ( f->str == "cons" || f->str == "define" || f->token.funcType == OPERATOR_FN || f->str == "and" || 
         f->str == "or" || f->token.funcType == EQU_TEST ) return 2 ;

    return 1 ; 
  } // GetProcedureFnArgs() 


  void DefineLambdaLocal( NodePtr para1, NodePtr para2, bool & error, string & errormsg, 
                          NodePtr & errorNode ) { // �w�qlambda���Ѽƭ� 
    int i = 0 ;

    NodePtr tempOut = NULL ;
    Symbol s ;
    bool useLocal = true ;
    NodePtr p2 = para2 ; // �Ȧspara2�@�}�l����m 
    
    // �������p��n�n�Q�w�q���ȫ�A�~�s�Jglocal�� 
    // �H�Uwhile �t�d�p��Ҧ��n�Q�w�q���� 
    while ( para2 != NULL && para2->token.type != NIL && error == false ) { 
      Eval( para2->left, tempOut, error, errormsg, errorNode, useLocal ) ; // �p���symbol��value
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
        if ( useLocal ) { // �d��ϰ��ܼ�
                    
          while ( i < glocal.size() ) {
            if ( in->token.str == glocal.at( i ).str ) { 
              out = glocal.at( i ).value ;
              find = true ;
            } // if 
      
            i++ ;
          } // while

        } // if

        i = 0 ; 
        if ( find == false ) { // �d������ܼ� 
          while ( i < gSymbolTAB.size() ) {
            if ( in->token.str == gSymbolTAB.at( i ).str ) { 
              out = gSymbolTAB.at( i ).value ;
            } // if 
      
            i++ ;
          } // while
        } // if
      } // if
      else { // �ëD�w�q��symbol 
        error = true ;
        errormsg = "ERROR (unbound symbol) : " + in->token.str + "\n" ;
      } // else 
    } // else if 
    else if ( in->left->token.funcType == CONS ) { // CONS (cons, list)  
      if ( in->left->token.str == "cons" ) { // cons
        in = in->right ;
        NodePtr temp = new Node() ;
        temp->left = in->left ; // �Ĥ@�ӰѼ� �񥪤l��
        in = in->right ;
        temp->right = in->left ; // �ĤG�ӰѼ� ��k�l��
        if ( IsATOM( in->left->token.type ) && in->left->token.type != NIL ) { // �Y�᭱��ATOM �nDOT 
          temp->str = "." ; // �����O�o���I 
          temp->token.type = DOT ;
        } // if 
        else { // �Y�᭱�OLIST �N���n�[DOT 
          temp->str = "(" ; // �����O�o���I 
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
      BeenQuoted( out ) ; // �N��ҫ��V�����ܦ�"" (�N�O���ӪF��)/ �����S��N�q 
    } // else if  QUOTE_FN
    else if ( in->left->token.funcType == DEFINE ) { // DEFINE
      out = new Node() ;
      out->left = NULL ;
      out->right = NULL ;
      in = in->right ;
      if ( in->left->token.type == SYMBOL ) { // is SYMBOL
        string symbol = in->left->str ; // symbol 
        out->str = symbol + " defined" ;
        NodePtr para1 = in->left ; // �Ĥ@�ӰѼƥ��H b 
        NodePtr para2 = in->right->left ; // �ĤG�ӰѼƥ��H a  
        in = in->right ;
        NodePtr temp = NULL ;
        if ( IsBoundFunction( para2->token ) ) { // ( define b a ) �Ba�O�۩w�q��function 
          out->token.type = STRING ;
          out->token.funcType = CUSTOMIZE ; 
          Fn f ;
          if ( IsBoundFunction( para1->token ) ) { // �Yb���w�Q�w�q��funciton�A�h�R�����s�w�q
            DeleteDefineFn( para1->str ) ; // �R�����e�w�q��function 
          } // if
          else if ( IsBoundSymbol( para1->token ) ) {
            DeleteDefineSym( para1->str ) ; // �R�����e�w�q��function 
          } // else if 

          f.str = para1->str ;
          if ( IsProcedureFn( para2->token ) ) { // �O�Ѥj�w�q��function 
            Symbol temps ;
            temps.str = f.str ;
            temps.value = para2 ;
            gSymbolTAB.push_back( temps ) ;

          } // if
          else { // �O�ڦۤv�w�q��function 
            f.value = GetFnDefine( para2->str ) ;
            f.originFn = GetOriginFn( para2->str ) ; // ���o�O�ѭ���function�ҩw�q�� 
            f.numOfArgs = GetFnArgsNum( para2->str, f.args ) ; 
            gFunctionTAB.push_back( f ) ;  // �N�w�q�n��function�]��JgFunctionTAB��
          } // else 

                            
        } // if // ( define b a ) �Ba�O�۩w�q��function 
        else { // �O�n�w�q��symbol 
          Symbol s ;
          s.str = symbol ;
          int index = 0 ;
          if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
            s.value = in->left ;                      
          } // if �Olambda 
          else { // ���Olambda                     
            Eval( in->left, temp, error, errormsg, errorNode, useLocal ) ; // Symbol�n�Q�w�q����
            out->token.type = STRING ;
            out->token.funcType = CUSTOMIZE ; 
            if ( error == false ) {          
              s.value = temp ;       
            } // if           
          } // else ���Olambda 

          if ( HasExistInSymbolTAB( s.str, index ) ) { // �w�g�s�b�bgSymbolTAB���F
            gSymbolTAB.insert( gSymbolTAB.begin() + index, s ) ; // ����i�h 
            gSymbolTAB.erase( gSymbolTAB.begin() + index + 1 ) ; // �R���쥻�� 
          } // if 
          else { // ���e�S�X�{�L�A �ҥH���J�̫᭱ 
            gSymbolTAB.push_back( s ) ; 
          } // else 
        } // else �O�n�w�q��symbol 
      } // if SYMBOL
      else { // define (F x ) ( s-exp )
        NodePtr bone = in ;
        in = in->left ; // (F  x y z)
        Fn temp_Fn ;
        int counter = 0 ; // �p��ѼƼƶq�� 
        if ( IsBoundFunction( in->left->token ) ) { // �Y���w�Q�w�q��funciton�A�h�R�����s�w�q 
          DeleteDefineFn( in->left->str ) ; // �R�����e�w�q��function 
        } // if 

        temp_Fn.str = in->left->str ; // Fn name
        out->str = temp_Fn.str + " defined" ;
        out->token.type = STRING ;
        out->token.funcType = CUSTOMIZE ;
        if ( gVerbose == false ) { // verbose�Ofalse �N���n�L 
          out = NULL ;
        } // if
 
        in = in->right ;
        while ( in != NULL && in->token.type != NIL ) { // �N�Ҧ�function���ѼƳ��s�_�� 
          counter++ ;
          temp_Fn.args.push_back( in->left->str ) ;
          in = in->right ;
        } // while 

        temp_Fn.numOfArgs = counter ;
        in = bone->right ; // ����Fn���w�q(�n�������)
        temp_Fn.value = in ;
        temp_Fn.value->token.funcType = CUSTOMIZE ;
        temp_Fn.originFn = temp_Fn.str ; // �O���function 
        gFunctionTAB.push_back( temp_Fn ) ; 
      } // else define (F x ) ( s-exp )
    } // else if DEFINE
    else if ( in->left->token.funcType == PART_ACCESSOR ) { // PART_ACCESSOR(car, cdr)
      if ( IsPair( in, errormsg ) ) {
        NodePtr temp = in->right->left ; // list��b����m(10)
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
      t->token.type = T ; // �D�`���n �@�w�n�]�wtype
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
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) ) { // true
          out = t ;
        } // else if true
        else { // �L���~�B��false 
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
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == NIL ) { // true
          out = t ;
        } // else if true
        else { // �L���~�B��false 
          out = f ;
        } // else false
      } // else if null?
      else if ( in->left->token.str == "integer?" ) { // integer?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == INT ) { // true
          out = t ;
        } // else if true
        else { // �L���~�B��false 
          out = f ;
        } // else false
      } // else if  integer?
      else if ( in->left->token.str == "real?" || in->left->token.str == "number?" ) { // real? number?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == INT || temp->token.type == FLOAT ) ) {
          out = t ;
        } // else if true
        else { // �L���~�B��false 
          out = f ;
        } // else false        
      } // else if real? number?
      else if ( in->left->token.str == "string?" ) { // string?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && temp->token.type == STRING ) { // true
          out = t ;
        } // else if true
        else { // �L���~�B��false 
          out = f ;
        } // else false
      } // else if  string?
      else if ( in->left->token.str == "boolean?" ) { // boolean?
        in = in->right ; // (1)
        temp = in->left ;
        if ( error ) { // ���ѼƦ����~ 
          ;
        } // if 
        else if ( IsATOM( temp->token.type ) && ( temp->token.type == T || temp->token.type == NIL ) ) {
          out = t ;
        } // else if true
        else { // �L���~�B��false 
          out = f ;
        } // else false
      } // else if  boolean?
      else if ( in->left->token.str == "symbol?"  ) { // symbol?
        in = in->right ; // (1)
        temp = in->left ;
        // if ( temp->token.type == SYMBOL || temp->token.funcType == BEEN_QUOTE ) { 
        // �]��'3 ���Osymbol�� 
        if ( temp->token.type == SYMBOL ) {
          out = t ;
        } // if true
        else { // �L���~�B��false 
          out = f ;
        } // else false
      } // else if symbol?
    } // else if PRIMITIVE_PREDICATE
    else if ( in->left->token.funcType == OPERATOR_FN ) { // OPERATOR_FN(+-*/)
      NodePtr t = new Node() ;
      t->str = "#t" ;
      t->token.type = T ; // �D�`���n �@�w�n�]�wtype
      t->left = NULL ;
      t->right = NULL ;
      NodePtr f = new Node() ;
      f->str = "nil" ;
      f->token.type = NIL ;
      f->left = NULL ;
      f->right = NULL ;

      // Token define_token = GetDefineToken( in->left->token ) ;
       
      if ( in->left->str == "+" ) { // + Add
        Add( in->right, out, error, errormsg, errorNode ) ; // ��ѼƳ��[�_�� �Y�����A�]�|�b�䤤�]�w�nerrormsg
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
      t->token.type = T ; // �D�`���n �@�w�n�]�wtype
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
        NodePtr para1 = in->left ; // �Ĥ@�ӰѼ�
        in = in->right ; 
        NodePtr para2 = in->left ; // �ĤG�ӰѼ�

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
      bool useLocal = true ; // �O�_�n��local variables 
      glocalTemp = glocal ;
      DefineSym( bone->left ) ; // �w�qlocal variables
      bone = bone->right ;
      while ( bone != NULL && bone->token.type != NIL ) { // �}�l����n������ 
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
        out->token.type = T ; // �D�`���n �@�w�n�]�wtype
        out->left = NULL ;
        out->right = NULL ;
      } // else 
    } // else if
    else if ( in->left != NULL && in->left->left != NULL && in->left->left->token.funcType == LAMBDA ) {
      // (( lambda XXXXX ...
      NodePtr root = in ; 
      NodePtr para2 = in->right ; // �n�Q�w�q���Ѽƭ� �|�Q���para2(para2���o�ǭȪ��Ҧbbone) 
      NodePtr para1 = in->left->right->left ; // �Ѽƪ�bone

      glocalTemp = glocal ;
      DefineLambdaLocal( para1, para2, error, errormsg, errorNode ) ;
      if ( error && errormsg == "ERROR (incorrect number of arguments) : " ) {
        errorNode = in->left->left ;
      } // if 

      in = root->left->right->right ; // lambda���̫�@�ӰѼ� �]�N�O�n���檺�a�� 
      while ( in != NULL && error == false ) {
        Eval( in->left, out, error, errormsg, errorNode, useLocal ) ;
        in = in->right ;
      } // while 

      glocal = glocalTemp ; // ���槹lambda �N�Nglocal�_�� 
    } // else if
    else if ( in->left->token.funcType == CUSTOMIZE ) {
      DefineLocal( in->left->str, in->right, useLocal, error, errormsg, errorNode ) ; 
      // ��function�W�A�Ѽ�(�_�l)�Ҧb��bone
      NodePtr fnDefine = GetFnDefine( in->left->str ) ; // ���o��function�n������ 
      useLocal = true ;
      if ( error == false ) { 
        Eval( fnDefine, out, error, errormsg, errorNode, useLocal ) ; // ���榹function 
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
    string str = "" ; // ���F�নstring�s�i(vector)gLine�� 
    bool exit = false ;
    bool error = false ; // ��Token�άO��k�P�_�ɡA���L���~ERROR
    string errormsg = "" ; 
    bool isComplete = true ; // �O�_������i�H�ؾ𪺪F�� 
    while ( exit == false && scanf( "%c", &ch ) != EOF  ) { // �Y�٨SŪ��

      if ( ch == '\n' ) { // ����]�n�s�i�h ���F�P�_ERROR�O�b�ĴX�� 
        str = str + ch ; // �নstring�s 
        gLine.push_back( str ) ;
        str = "" ;
        GetToken( error ) ; // �h��token�A����
        gLine.clear() ; 

        // �X�ӫ� ���������F
        // �h��k�P�_ �A�Y����٤����N��Ū�U�@�� 
        error = false ;
        int index = 0 ; // gToken��index
        isComplete = true ;
        bool isFirstToken = true ;
        bool isEOF = false ;
        bool hasQuote = false ;
        SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

        if ( error ) { // ��k���~�LERROR 
          cout << errormsg ;
          int k = 0 ;
          while ( ! gToken.empty() && k <= index ) { // �M������a��   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

            
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // �M���o�@�� 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
            
          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // �M������ 

          gLine.clear() ;
          cout << "\n> " ; 
        } // if 
        else { // ��k�L�~ 
          if ( IsExitToken() ) exit = true ;
            
          gTree = NULL ;
          NodePtr tree = gTree ;
          int i = 0 ; // index of token
          bool isLeftChild = false ;
          bool isRightChild = false ;
          int lastIndexOfToken = -1 ; // �O�ؾ�gToken�����
          int lp = 0 ; // ���A����
          int rp = 0 ; // �k�A���� ���k�A���ƹ�١A�h��ܦ��\�ؾ�
          string parent = "" ;
          int limitIndex = index ; // �ؾ�u��ب�o��
          bool useLocal = false ; // �O�_�ϥ�local Variables 
          if ( isComplete ) {
            if ( hasQuote ) { // ��Quote�~�ݭn�bQuote�e��[���A�� 
              int count = SortOutQuote( limitIndex ) ;
              limitIndex = count + limitIndex ;
            } // if 
            
            BuildTree( tree, i, isLeftChild, isRightChild, lastIndexOfToken, lp, rp, 
                       parent, limitIndex ) ;
            // PrettyPrint( gTree, error ) ;
            tree = NULL ;               
            tree = gTree ;
            NodePtr out = NULL ;
            NodePtr errorNode  = NULL ; // �����~���n�Ls-exp�A�s�b�o�� 
            // result(pointer)���V�n�L���F��(EVAL�����G) 
            Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
            if ( IsExitTree( tree ) ) {
              exit = true ; // ���� 
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
            DeleteGToken( limitIndex ) ; // �M���@������gToken
            delete gTree ;
            gTree = NULL ;
            glocal.clear() ;
            if ( exit == true ) ;
            else cout << "\n> " ;  
          } // if          
        } // else ��k�L�~            

        error = false ; // ��serror (init)
        errormsg = "" ;     
      } // if       
      else {
        str = str + ch ; // �নstring�s 
        gLine.push_back( str ) ;
        str = "" ;  
      } // else 
    } // while 


    /*
    if ( gLine.size() != 0 && gLine.at( gLine.size() - 1 ) != "\n" ) {

      GetToken( error ) ; // �h��token�A����

      // �X�ӫ� ���������F
      // �h��k�P�_ �A�Y����٤����N��Ū�U�@�� 
      if ( error ) {
        gToken.clear() ;
        cout << "\n> " ;
      } // if
    } // if  
    */
    
    GetToken( error ) ; // �h��token�A����
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
      int index = 0 ; // gToken��index 
      isComplete = true ;
      error = false ;
      bool isFirstToken = true ;
      bool isEOF = false ;
      bool hasQuote = false ; 
      SyntaxAnalysis( index, error, errormsg, isComplete, isFirstToken, isEOF, hasQuote ) ;

      if ( error ) { // ��k���~�LERROR  
        if ( isEOF ) { // ���� 
          gToken.clear() ; 
        } // if 
        else {
          cout << errormsg ;
          int k = 0 ;
        
          while ( k <= index ) { // �M������a��   
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while

          
          k = 0 ;
          while ( ! gToken.empty() && gToken.at( 0 ).str != "\n" ) { // �M���o�@�� 
            gToken.erase( gToken.begin() ) ;
            k++ ;
          } // while 
          

          if ( ! gToken.empty() && gToken.at( 0 ).str == "\n" )  
            gToken.erase( gToken.begin() ) ; // �M������ 

          cout << "\n> " ;
          errormsg = "" ;
        } // else 
      } // if ��k���~�LERROR 
      else { // ��k�L�~
        if ( IsExitToken() ) end = true ;
        
        if ( end == true ) gToken.erase( gToken.end() ) ;
        gTree = NULL ;
        NodePtr tree = gTree ; 
        int i = 0 ; // index of token
        bool isLeftChild = false ;
        bool isRightChild = false ;
        int lastIndexOfToken = -1 ; // �O�ؾ�gToken�����
        int lp = 0 ; // ���A����
        int rp = 0 ; // �k�A���� ���k�A���ƹ�١A�h��ܦ��\�ؾ�
        string parent = "" ;
        int limitIndex = index ; // �ؾ�u��ب�o��
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
          NodePtr errorNode  = NULL ; // �����~���n�Ls-exp�A�s�b�o�� 
          // result(pointer)���V�n�L���F��(EVAL�����G) 
          Eval( tree, out, error, errormsg, errorNode, useLocal ) ;
          if ( IsExitTree( tree ) ) {
            exit = true ; // ���� 
          } // if 
          else if ( error && errorNode == NULL ) cout << errormsg ;
          else if ( error ) {
            cout << errormsg ;
            // �]��non-function �n�L�X#<Procedure> �o�� �ҥH�]�w��no error ���I���T�w! 
            if ( errormsg == "ERROR (attempt to apply non-function) : " ) error = false ;
            PrettyPrint( errorNode, error ) ;  
          } // else if 
          else {
            PrettyPrint( out, error ) ;
          } // else 

          if ( isFirstToken ) limitIndex-- ; 
          DeleteGToken( limitIndex ) ; // �M���@������gToken
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
