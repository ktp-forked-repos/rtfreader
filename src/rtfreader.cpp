/*
CSTRTFREADER - read flat text or rtf and output flat text, 
               one line per sentence, optionally tokenised

Copyright (C) 2012  Center for Sprogteknologi, University of Copenhagen

This file is part of CSTRTFREADER.

CSTRTFREADER is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

CSTRTFREADER is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CSTRTFREADER; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*

segment[Text]                                                       Recursively traverse RTF segment tree.
  |                                                                 Keep track of character properties (bold, italic,
  |                                                                 small caps, font size)
  |                                                                 Returns true if segment is a field. False otherwise.
  |                                                                 Sets begin and end offsets for candidate segments, 
  |                                                                 based on the interpretation of RTF tags such as \par
  |
  |                                                                 In case of flat text input: detect well-formed HTML-tags.
  |
  +--segmentdefault[Text]                                           Tries to spot sentence endings by looking at interpunction.
       |                                                                Knows of filenames, ellipsis ...
       +--doTheSegmentation[Text]
            | |
            | +--GetPutBullet                                       Skip rest of segment. Print bullet character and a blank.
            |               |
            +--GetPut[Text] |                                       Read segment. Convert rtf-tokens to their character equivalents.
            |     |         |                                       Suppress optional hyphens.
            |     |         |
            +-----+---------+--PutHandlingLine                      Handle a line, detecting noise
                                 |
                                 +--PutHandlingWordWrap             Handle hyphens at end of line
                                 |    |
                                 +----+--Put                        Handle newlines
                                           |
                                           +--Put2                  Handle quotes, brackets and abbreviations
                                                |
                                                +--PutN             Repeated call of Put3 on whole buffer
                                                |    |
                                                +----+--Put3        Start new sentence if an abbreviation is followed by a capital 
                                                          |         letter in a new segment
                                                          |
                                                          +--Put4   Optionally translate 8-bit set characters to combinations 
                                                                    of 7-bit characters
*/
/*This source is also used in OCRtidy*/

#include "rtfreader.h"
#include "unicode.h"
#include "commonstuff.h"
#include "rtfstuff.h"
#include "txtstuff.h"
#include <string.h>
#include "option.h"
#if FILESTREAM
#include "abbrev.h"
#include "mwu.h"
#endif
bool doit(STROEM * sourceFile, STROEM * targetFile)
    {
//    test = fopen("test","w");
//TODO    readArgs(argc, argv,Option);
    if(Option.A)
        pPut4 = Put4A;
    
    bool doneOK = false;
    if(Option.x == '+')
        {
        char buf[6];
        for(int j = 0;j < 5;++j)
            buf[j] = (char)Getc(sourceFile);
        Frewind(sourceFile);
        buf[5] = '\0';
        if(strcmp(buf,"{\\rtf"))
            {
            doneOK = TextSegmentation(sourceFile, targetFile);
            }
        else
            {
            Option.encoding = eUTF8;
            forceOutputUnicode(&Fputc,Option.encoding);
            doneOK = RTFSegmentation(sourceFile, targetFile);
            }
        }
    else if(Option.x == 0)
        {
        doneOK = TextSegmentation(sourceFile, targetFile);
        }
    else
        {
        doneOK = RTFSegmentation(sourceFile, targetFile);
        }
#if FILESTREAM
    if(doneOK)
        {
        if(!DoMultiWords())
            return false;
        }
#endif
//    fclose(test);
    return true;
    }