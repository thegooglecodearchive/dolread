#include <stdio.h>
#include <string.h>
#include "dfconv.h"

int dfConvert( char * Buffer, int iLen ) {
  int in_label=0;
  int in_script=0;
  int in_style=0;
  int in_pre=0;
  int  iPos,iPointer;
  char c;

  iPos=iPointer=0;
  while( iPointer < iLen) {
    if (! in_pre) { //only keep spaces when not in <pre> scope
      while (iPointer < iLen && (Buffer[iPointer] == ' ' || Buffer[iPointer] == '\t' || Buffer[iPointer] == '\r' || Buffer[iPointer] == '\n')) {
        iPointer++;
      }
      if (iPointer == iLen) {
        break;
      }
    }
    iPointer++;

    if (Buffer[iPointer - 1] == '<' && Buffer[iPointer] == 's') {
      iPointer = iPointer;
    }

    //p
    if( dfCheckChar( iLen, Buffer, iPointer, "<P>", 3 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=2;
      continue;
    }

    if( dfCheckChar( iLen, Buffer, iPointer, "</P>", 4 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=3;
      continue;
    }

    //hr
    if( dfCheckChar( iLen, Buffer, iPointer, "<HR>", 4 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=3;
      continue;
    }

    //center
    if( dfCheckChar( iLen, Buffer, iPointer, "<CENTER>", 8 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=7;
      continue;
    }

    if( dfCheckChar( iLen, Buffer, iPointer, "</CENTER>", 9 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=8;
      continue;
    }

    //title
    if( dfCheckChar( iLen, Buffer, iPointer, "</TITLE>", 8 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=7;
      continue;
    }

    //style
    if( dfCheckChar( iLen, Buffer, iPointer, "<STYLE", 6 ) ) {
      in_style=1;
      iPointer+=5;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</STYLE>", 8 ) ) {
      in_style=0;
      iPointer+=7;
      continue;
    }

    //pre
    if( dfCheckChar( iLen, Buffer, iPointer, "<PRE>", 5 ) ) {
      in_pre=1;
      iPointer+=4;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</PRE>", 6 ) ) {
      in_pre=0;
      iPointer+=5;
      continue;
    }

    //script
    if( dfCheckChar( iLen, Buffer, iPointer, "<SCRIPT", 7 ) ) {
      in_script=1;
      iPointer+=6;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</SCRIPT>", 9 ) ) {
      in_script=0;
      iPointer+=8;
      continue;
    }

    // BlockQuote
    if( dfCheckChar( iLen, Buffer, iPointer, "<BLOCKQUOTE>", 12 ) ) {
      Buffer[iPos++]='"';
      iPointer+=11;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</BLOCKQUOTE>", 13 ) ) {
      Buffer[iPos++]='"';
      iPointer+=12;
      continue;
    }

    // LineBreak
    if( dfCheckChar( iLen, Buffer, iPointer, "<BR>", 4 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=3;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</BR>", 5 ) ) {
      Buffer[iPos++]='\n';
      iPointer+=4;
      continue;
    }

    // Citation
    if( dfCheckChar( iLen, Buffer, iPointer, "<CITE>", 6 ) ) {
      Buffer[iPos++]='"';
      iPointer+=5;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</CITE>", 7 ) ) {
      Buffer[iPos++]='"';
      iPointer+=6;
      continue;
    }

    // Tab
    if( dfCheckChar( iLen, Buffer, iPointer, "<TD>", 4 ) ) {
      Buffer[iPos++]=9;
      iPointer+=3;
      continue;
    }
    if( dfCheckChar( iLen, Buffer, iPointer, "</TD>", 5 ) ) {
      Buffer[iPos++]=9;
      iPointer+=4;
      continue;
    }

    // HTML Command Skipper
    if( Buffer[iPointer-1]=='<' ) {
      if( (c = Buffer[iPointer]) !='\0' ) {
        if( (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <='Z' )
            || (c == '!') || (c == '/')
          ) {
          in_label=1;
        }
      }
    }
    if( Buffer[iPointer-1]=='>'&& in_label ) {
      in_label=0;
      continue;
    }

    if( !in_label) {
      if( Buffer[iPointer-1]=='&' ) {
        if( dfCheckChar( iLen, Buffer, iPointer, "&lt;"     , 4 ) ) {
          Buffer[iPos++]='<';
          iPointer+=3;
          continue;
        } //4
        if( dfCheckChar( iLen, Buffer, iPointer, "&gt;"     , 4 ) ) {
          Buffer[iPos++]='>';
          iPointer+=3;
          continue;
        } //4
        if( dfCheckChar( iLen, Buffer, iPointer, "&amp;"    , 5 ) ) {
          Buffer[iPos++]='&';
          iPointer+=4;
          continue;
        } //5
        if( dfCheckChar( iLen, Buffer, iPointer, "&quot;"   , 6 ) ) {
          Buffer[iPos++]='"';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&Aacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Agrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Acirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Atilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Aring;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Auml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&AElig;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ccedil;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Eacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Egrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ecirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Euml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&Iacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Igrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Icirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Iuml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&ETH;"    , 5 ) ) {
          Buffer[iPos++]='?';
          iPointer+=4;
          continue;
        } //5
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ntilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Oacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ograve;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ocirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Otilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ouml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&Oslash;" , 8 ) ) {
          Buffer[iPos++]='0';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Uacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ugrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&Ucirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&Uuml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&Yacute;" , 8 ) ) {
          Buffer[iPos++]='Y';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&THORN;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&szlig;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&aacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&agrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&acirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&atilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&aring;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&auml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&aelig;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&ccedil;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&eacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&egrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ecirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&euml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&iacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&igrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&icirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&iuml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&eth;"    , 5 ) ) {
          Buffer[iPos++]='?';
          iPointer+=4;
          continue;
        } //5
        if( dfCheckChar( iLen, Buffer, iPointer, "&ntilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&oacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ograve;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ocirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&otilde;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ouml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&oslash;" , 8 ) ) {
          Buffer[iPos++]='0';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&uacute;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ugrave;" , 8 ) ) {
          Buffer[iPos++]='?';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&ucirc;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&uuml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&yacute;" , 8 ) ) {
          Buffer[iPos++]='Y';
          iPointer+=7;
          continue;
        } //8
        if( dfCheckChar( iLen, Buffer, iPointer, "&thorn;"  , 7 ) ) {
          Buffer[iPos++]='?';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&yuml;"   , 6 ) ) {
          Buffer[iPos++]='?';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&reg;"    , 5 ) ) {
          Buffer[iPos++]='(';
          Buffer[iPos++]='r';
          Buffer[iPos++]=')';
          iPointer+=4;
          continue;
        } //5
        if( dfCheckChar( iLen, Buffer, iPointer, "&copy;"   , 6 ) ) {
          Buffer[iPos++]='(';
          Buffer[iPos++]='c';
          Buffer[iPos++]=')';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&trade;"  , 7 ) ) {
          Buffer[iPos++]='t';
          Buffer[iPos++]='m';
          iPointer+=6;
          continue;
        } //7
        if( dfCheckChar( iLen, Buffer, iPointer, "&nbsp;"   , 6 ) ) {
          Buffer[iPos++]=' ';
          iPointer+=5;
          continue;
        } //6
        if( dfCheckChar( iLen, Buffer, iPointer, "&middot;"   , 8 ) ) {
          //middle dot in GBK code
          Buffer[iPos++]=(char)0xa1;
          Buffer[iPos++]=(char)0xa4;
          iPointer+=7;
          continue;
        }

        // &#number
        if( Buffer[iPointer]=='#' ) {
          // May be a Number
          int nCount = 0;
          while( iLen-(nCount+1)>0              && // I have char?
                 Buffer[iPointer+1+nCount]>='0' && // or number ?
                 Buffer[iPointer+1+nCount]<='9' ) {
            nCount++;
          }

          // If I have number .. try to convert it
          if( nCount>0 ) {
            int nDmm  = 0;
            int nChar = 0;
            int nMul  = 1;
            while( nDmm<nCount ) {
              nChar += (Buffer[iPointer+nCount-nDmm]-'0')*nMul;
              printf( "%d\n", Buffer[iPointer+nCount-nDmm]-48 );
              printf( "%d\n", nMul );
              nMul  *= 10;
              nDmm++;
            }
            if( nChar>0 ) {
              Buffer[iPos++]=nChar;
              iPointer+=nDmm+1;
              continue;
            }
          }
        }
      }

#if 0
      if( Buffer[iPointer-1]==0x0d &&
          Buffer[iPointer  ]==0x0d ) {
        iPointer++;
        continue;
      }
#endif

      if (! in_script && ! in_style) {
        Buffer[iPos++]=Buffer[iPointer-1];
      }
    } //if (! in_label)
  }
  return iPos;
}

// check string
int dfCheckChar( int iLen,
                 char *Buffer,
                 int iPointer,
                 char *Check,
                 int iCheckLen ) {
  if( iLen - iPointer +1 >= iCheckLen ) {
    iPointer--;
    return _strnicmp(Buffer + iPointer, Check, iCheckLen) == 0;
  }

  return 0;
}
