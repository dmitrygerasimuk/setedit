/* Copyright (C) 1996-2007 by Salvador E. Tropea (SET),
   see copyrigh file for details */
// That's the first include because is used to configure the editor.
#include <ceditint.h>

#define Uses_stdlib
#define Uses_stdio
#define Uses_string
#define Uses_AllocLocal

#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TProgram
#define Uses_TApplication
#define Uses_TObject
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_THistory
#define Uses_TRect
#define Uses_TCheckBoxes
#define Uses_TRadioButtons
#define Uses_TButton
#define Uses_MsgBox
#define Uses_TSItem
#define Uses_TStringCollection
#define Uses_fpstream
#define Uses_TKeys
#define Uses_TStaticText
#define Uses_TCEditor_Internal
#define Uses_TCEditWindow
#define Uses_TInputLinePiped
#define Uses_TCEditor_Commands

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSHzGroup
#define Uses_TSInputLinePiped
#define Uses_TSInputLine
#define Uses_TSCheckBoxes
#define Uses_TSRadioButtons
#define Uses_TSLabel
#define Uses_TSSortedListBox
#define Uses_TSVeGroup
#define Uses_TSLabelCheck
#define Uses_TSLabelRadio
#define Uses_TSStringableListBox

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#include <edhists.h>
#include <diaghelp.h>
#include <setconst.h>
#include <editcoma.h>

#include <stdarg.h>

#define Uses_SETAppConst
#include <setapp.h>

/**[txh]********************************************************************

  Description:
  This dialog is used to configure the RegEx options. They are: style
(basic,extended and perl), replace (normal or tagged) and optimize (allow
optimization of the regex when the search isn't a regex).

***************************************************************************/

TDialog *createRegExOpsDialog()
{
 TSViewCol *col=new TSViewCol(__("Regular Expression Options"));

 // EN: ABDENPT
 TSLabel *Style=TSLabelRadio(__("RegEx style"),__("~B~asic POSIX"),
                             __("~E~xtended POSIX"),
                             SUP_PCRE ? __("~P~erl Compatible") : 0,
                             NULL);

 TSLabel *Replace=TSLabelRadio(__("Replace text"),__("~N~ormal text"),
                               __("~D~ollar tags"),NULL);
 TSLabel *Optimize=TSLabelRadio(__("Optimize"),__("~T~ry to use normal search"),
                                __("~A~lways use RegEx"),NULL);

 TSVeGroup *rightG=new TSVeGroup(Replace,Optimize);
 rightG->makeSameW();

 col->insert(xTSLeft,yTSUp,Style);
 col->insert(xTSRightOf,yTSUp,rightG,Style);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(hcRegExOptions);
 delete col;
 return d;
}

static void *boxRegEx;

static
int RegExDialog(unsigned , void *)
{
 execDialog(createRegExOpsDialog(),boxRegEx);
 return btcbGoOn;
}

// Eng: BCDEFGHIKOSTUWX
// Esp: ABCDFGHIKLMOPSTX
TDialog *createFindDialog(void *regexBox)
{
 boxRegEx=regexBox;
 TSViewCol *col=new TSViewCol(__("Find"));

 TSVeGroup *Options=
 MakeVeGroup(0, // All together
             new TSLabel(__("~T~ext to find"),
                 new TSInputLinePiped(maxFindStrLenEd,1,hID_TextSearchEditor,
                                      GetDeskTopCols()/2+2)),
             TSLabelCheck(__("Options"),__("~C~ase sensitive"),
                          __("~W~hole words only"),__("Regular e~x~pressions"),
                          __("Only ~i~nside comments"),__("Only o~u~tside comments"),
                          __("S~h~ow function name"),NULL),
             TSLabelRadio(__("Scope"),__("~G~lobal"),__("~S~elected text"),NULL),
             TSLabelRadio(__("Origin"),__("~F~rom cursor"),
                          __("~E~ntire scope"),NULL),
             TSLabelRadio(__("Direction"),__("Forwar~d~"),
                          __("~B~ackward"),NULL),
             NULL);
 Options->makeSameW();

 TSHzGroup *but123=MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                               new TSButton(__("Cancel"),cmCancel),
                               new TSButton(__("RegEx ~O~ps"),cmRegExOptions,
                                            bfNormal,RegExDialog),
                               NULL);

 col->insert(xTSCenter,yTSUp,Options);
 col->insert(xTSCenter,yTSDown,but123);

 TDialog *d=col->doItCenter(cmcFind);
 delete col;
 return d;
}


// Eng: ABCDEFGHIKNOPRSTWX
// Esp: ABCDEFGHKLMNOPRSTUX
TDialog *createReplaceDialog(void *regexBox)
{
 boxRegEx=regexBox;
 TSViewCol *col=new TSViewCol(__("Replace"));
 int FixWTest=(GetDeskTopCols()-12)/2;

 TSLabel *TextToFind=new TSLabel(__("~T~ext to find"),
                       new TSInputLinePiped(maxFindStrLenEd,1,hID_TextSearchEditor,FixWTest));

 TSLabel *newText=new TSLabel(__("~N~ew text"),
                    new TSInputLinePiped(maxReplaceStrLenEd,1,hID_TextReplaceEditor,FixWTest));

 TSLabel *Options=TSLabelCheck(
                  __("Options"),__("~C~ase sensitive"),__("~W~hole words only"),
                  __("Regular e~x~pressions"),__("Only ~i~nside comments"),
                  __("Only o~u~tside comments"),__("S~h~ow function ~n~ame"),
                  __("~P~rompt on replace"),__("~R~eplace all"),NULL);
 Options->setWidth(FixWTest);

 TSVeGroup *OriScoDir=
 MakeVeGroup(tsveMakeSameW, // All together + same width
             TSLabelRadio(__("Scope"),__("~G~lobal"),__("~S~elected text"),NULL),
             TSLabelRadio(__("Or~i~gin"),__("~F~rom cursor"),__("~E~ntire scope"),NULL),
             TSLabelRadio(__("Direction"),__("Forwar~d~"),__("~B~ackward"),NULL),
             NULL);
 TSView::makeSameW(newText,OriScoDir);

 TSHzGroup *but123=MakeHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                               new TSButton(__("Replace ~A~ll"),cmYes),
                               new TSButton(__("Cancel"),cmCancel),
                               new TSButton(__("RegEx ~O~ps"),cmRegExOptions,
                                            bfNormal,RegExDialog),
                               NULL);

 col->insert(xTSLeft,yTSUp,TextToFind);
 col->insert(xTSRightOf,yTSUp,newText,TextToFind);
 col->insert(xTSLeft,yTSUnder,Options,0,TextToFind);
 col->insert(xTSRightOf,yTSUnder,OriScoDir,TextToFind,newText);
 col->insert(xTSCenter,yTSDown,but123);

 TDialog *d=col->doItCenter(cmcReplace);
 delete col;
 return d;
}


TDialog *createGotoLineDialog()
{
 TSViewCol *col=new TSViewCol(__("Goto Line"));

 TSInputLine *lineInp=new TSInputLine(10,(ushort)hID_GotoLine);
 TSHzLabel *lineLabel=new TSHzLabel(__("Desired line"),lineInp);

 col->insert(xTSCenter,yTSUpSep,lineLabel);
 EasyInsertOKCancel(col,3);

 TDialog *d=col->doItCenter(cmcGotoEditorLine);
 delete col;
 return d;
}

// Syntax Highlight options is a subdialog of local options
static ShlDiagBox *SHL_Box;

static
int SHLSubDiag(unsigned , void *)
{
 TSViewCol *col=new TSViewCol(__("Syntax Highlighting"));
 
 #define VeSHLW 30
 int VeSHLH=GetDeskTopRows()-15;
 TSRadioButtons *Shl=new TSRadioButtons(
     new TSItem(__("Off ~1~"),
     new TSItem(__("C++ st~y~le"),
     new TSItem(__("Pa~s~cal style"),
     new TSItem(__("C~l~ipper style"),
     new TSItem(__("User ~d~efined"),0))))),VeSHLW);
 TSSortedListBox *ListaH=new TSSortedListBox(VeSHLW,VeSHLH,tsslbVertical);
 TSVeGroup *ShlVe=new TSVeGroup(Shl,ListaH,0);
 
 col->insert(xTSCenter,yTSUp,ShlVe);
 EasyInsertOKCancel(col);
 
 TDialog *d=col->doItCenter(cmcSetLocalOptions);
 delete col;

 ShlDiagBox temp;
 memcpy(&temp,SHL_Box,sizeof(ShlDiagBox));
 if (execDialog(d,&temp)!=cmCancel)
    memcpy(SHL_Box,&temp,sizeof(ShlDiagBox));

 return btcbGoOn;
}

TDialog *createSetLocalOptions(ShlDiagBox *shlBox)
{ // English: ABCDEFGHIKLMNOPRSTUWYZ 1234
  // BEGJQVXZ
 TSViewCol *col=new TSViewCol(__("Local Options"));
 SHL_Box=shlBox;

 TSLabel *Options=TSLabelCheck(2,__("Options ~3~"),__("~O~verwrite"),__("~A~utoindent"),
         __("~U~se real tabs (ASCII 9)"),__("~P~ersistent blocks"),__("~I~ntelligent indent"),
         __("~C~olumn cursor"),__("~R~ow cursor"),__("~M~atched pair highlighting"),
         __("Match pair on the fl~y~"),__("Don't wait to search for the pair ~1~"),
         __("Tra~n~sparent Blocks"),__("Optimal ~F~ill"),__("~W~rap words"),
         __("S~e~e tabs"),__("Don't move inside ta~b~s"),
         __("Tab ~s~mart indents"),__("Use in~d~ent size"),
         __("Keep trailin~g~ whitespace"),__("Backspace unindents ~2~"),
         __("Column markers ~4~"),NULL);

 TSHzGroup *Inputs=MakeHzGroup(3,
            new TSHzLabel(__("~T~ab size"),new TSInputLine(3)),
            new TSHzLabel(__("Indent si~z~e"),new TSInputLine(3)),
            new TSHzLabel(__("Wrap co~l~umn"),new TSInputLine(4)),
            NULL);
 TSHzLabel *colMarkers=new TSHzLabel(__("Column markers"),
                                     new TSInputLine(colMarkersStrLen,30));
 TSVeGroup *Ops=MakeVeGroup(0,Options,Inputs,colMarkers,NULL);

 TSHzGroup *but123=MakeHzGroup(
            new TSButton(__("O~K~"),cmOK,bfDefault),
            new TSButton(__("Cancel"),cmCancel),
            new TSButton(__("Syntax ~H~L Options"),cmSHLOptions,bfNormal,SHLSubDiag),
            NULL);
            
 col->insert(xTSCenter,yTSUp,Ops);
 col->insert(xTSCenter,yTSDown,but123);

 TDialog *d=col->doItCenter(cmcSetLocalOptions);
 delete col;
 return d;
}


// Shortcuts: English: ABCDEFGHIKLMNOPRSTUWYZ 5689
TDialog *createSetGlobalOptions()
{
 TSViewCol *col=new TSViewCol(__("Global Options"));

 TSLabel *Options=TSLabelCheck(2,__("Options"),
          __("~A~utoindent"),
          __("~U~se tabs"),
          __("~P~ersistent blocks"),
          __("~I~ntelligent indent"),
          __("~C~olumn cursor"),
          __("~R~ow cursor"),
          __("~M~atched pair highlighting"),
          __("Match pair on the fl~y~"),
          __("Don't wait to searc~h~ for the pair"),
          __("~D~on't move the cursor on Paste"),
          __("Tra~n~sparent Blocks"),
          __("Optimal ~F~ill"),
          __("~W~rap words"),
          __("~S~croll Lock centers"),
          __("S~e~e tabs"),
          __("Don't move inside ta~b~s"),
          __("Tab smart indents ~5~"),
          __("Use indent size ~6~"),
          __("Keep trailin~g~ whitespace"),
          __("Backspace unindents ~8~"),
          __("Column markers ~9~"),
         NULL);
 TSHzGroup *Inputs=MakeHzGroup(3,
            new TSHzLabel(__("~T~ab size"),new TSInputLine(3)),
            new TSHzLabel(__("Indent si~z~e"),new TSInputLine(3)),
            new TSHzLabel(__("Wrap co~l~umn"),new TSInputLine(4)),
            NULL);
 TSHzLabel *colMarkers=new TSHzLabel(__("Column markers"),
                                     new TSInputLine(colMarkersStrLen,30));
 TSVeGroup *Ops=MakeVeGroup(0,Options,Inputs,colMarkers,NULL);

 TSHzGroup *but123=MakeHzGroup(new TSButton(__("T~o~ all"), cmYes),
                               new TSButton(__("O~K~"), cmOK, bfDefault),
                               new TSButton(__("Cancel"), cmCancel),
                               NULL);

 col->insert(xTSCenter,yTSUp,Ops);
 col->insert(xTSCenter,yTSDown,but123);

 TDialog *d=col->doItCenter(cmcSetGlobalOptions);
 delete col;
 return d;
}

TDialog *createYesNoAllCancel(TPoint *size, TPoint *cursor)
{
 TSViewCol *col=new TSViewCol(__("Search hit"));

 TSStaticText *text=new TSStaticText(__("Replace this occurence?"));
 TSHzGroup *buttons=MakeHzGroup(new TSButton(__("~Y~es"),cmYes,bfDefault),
                                new TSButton(__("~N~o"),cmNo),
                                new TSButton(__("~A~ll"),cmOK),
                                new TSButton(__("Cancel"),cmCancel),
                                NULL);

 col->insert(xTSCenter,yTSUpSep,text);
 col->insert(xTSCenter,yTSUnder,buttons,0,text);
 TDialog *d=col->doIt();
 delete col;

 // Avoid placing the dialog over the cursor
 int xOff=(size->x-d->size.x)/2;
 int yOff=(size->y-d->size.y)/2;
 // +1 because the shadow
 if (yOff<=cursor->y && yOff+d->size.y+1>=cursor->y)
    yOff=cursor->y+1;
 d->moveTo(xOff,yOff);
 return d;
}


TDialog *createHTMLExportOps()
{// BCDFLMO
 TSViewCol *col=new TSViewCol(__("Export Options"));

 TSVeGroup *Ops=new TSVeGroup(
                TSLabelCheck(__("~O~ptions"),
                             __("~F~ile name as title"),
                             __("Same ~b~ackground color as the editor"),
                             __("~M~onospaced font"),
                             __("Bo~l~d attribute"),
                             __("~U~se CSS and HTML 4.01"),NULL),
                TSLabelRadio(__("Colors"),__("Use ~c~olors"),
                             __("~D~on't use colors"),NULL)
                );

 Ops->makeSameW();

 col->insert(xTSCenter,yTSUp,Ops);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmeExportAsHTML);
 delete col;
 return d;
}

TDialog *createPMChoose()
{// BFLMO
 TSViewCol *col=new TSViewCol(__("Pseudo Macros"));

 col->insert(xTSCenter,yTSUp,new TSSortedListBox(40,GetDeskTopRows()-9,tsslbVertical));
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmcChoosePMacrosList);
 delete col;
 return d;
}

TDialog *createArbitraryIndent(int len)
{
 TSViewCol *col=new TSViewCol(__("Arbitrary indent"));

 TSLabel *label=new TSLabel(__("Indentation text"),
                  new TSInputLinePiped(len,1,hID_ArbitraryIndent,GetDeskTopCols()-20));
 col->insert(xTSCenter,yTSUp,label);
 EasyInsertOKCancel(col);

 TDialog *d=col->doItCenter(cmcArbitraryIndent);
 delete col;
 return d;
}

unsigned LimitedFileNameDialog(unsigned flags, const char *format, const char *file)
{
 char b[100];
 int l=strlen(file); 
 if (l>90)
   {
    strcpy(b,"~");
    strcat(b,file+l-90);
   }
 else
   strcpy(b,file);

 return messageBox(flags,format,b);
}

void ShowSavePoint(const char *file)
{
 LimitedFileNameDialog(mfInformation|mfOKButton,__("Data saved to file: %s"),file);
}

TDialog *createSolveModifCollision(Boolean haveDiff)
{
 TSViewCol *col=new TSViewCol(__("Solve collision"));

 col->insert(xTSCenter,yTSUp,
  MakeVeGroup(tsveMakeSameW,
              new TSStaticText(__("Problem:\nThe copy in memory is also modified\n")),
              new TSButton(__("~L~oad file from disk (discard changes)"),cmOK,bfDefault),
              new TSButton(__("~A~bort operation"),cmCancel),
              haveDiff ? new TSButton(__("Load and ~s~how differences"),cmYes) : 0,
              haveDiff ? new TSButton(__("~D~on't load and show differences"),cmNo) : 0,
              NULL));
 TDialog *d=col->doItCenter(cmeSetModiCkOps);
 delete col;
 return d;
}

/**[txh]********************************************************************

  Description:
  Creates a dialog containing the variables from a pmacro. The
@var{varsVals} is newly allocated buffer containing @var{nVars} elements of
MaxVarValLen bytes. The labels for the input boxes are collected from
@var{vars} and its maximum length must be indicated in @var{mLenVar}. The
name of the dialog should be provided in @var{name}.

  Return: False if the user aborted the macro.

***************************************************************************/

Boolean AskForPMVars(char *&varsVals, TNSCollection *vars, unsigned &nVars,
                     unsigned mLenVar, const char *name, TNSCollection *defs)
{// Default values
 nVars=0;
 varsVals=NULL;
 if (!vars) // No vars, go on
    return True;
 // How many?
 nVars=vars->getCount();
 if (!nVars) // No vars, go on
    return True;
 // Buffer for the values
 varsVals=new char[MaxVarValLen*nVars];
 memset(varsVals,0,MaxVarValLen*nVars);
 // Buffer for the labels
 AllocLocalStr(auxName,mLenVar+1);
 // Dialog
 TSViewCol *col=new TSViewCol(name ? name : "");
 TSView *tsv=NULL;
 int w=GetDeskTopCols()-8-mLenVar;
 if (w<10) w=10;
 for (unsigned i=0; i<nVars; i++)
    {
     TSInputLinePiped *lineInp=new TSInputLinePiped(MaxVarValLen,w);
     // Pad with spaces
     char *o=(char *)vars->at(i);
     unsigned offset=0;
     for (; offset<mLenVar; offset++)
        {
         if (*o)
           {
            auxName[offset]=*o;
            o++;
           }
         else
            auxName[offset]=' ';
        }
     auxName[offset]=0;
     stTVIntl *intlCache=TVIntl::dontTranslateSt();
     TSHzLabel *lineLabel=new TSHzLabel(auxName,lineInp,intlCache);
     lineLabel->ySep=0;
     if (!tsv)
        col->insert(xTSLeft,yTSUp,lineLabel);
     else
        col->insert(xTSLeft,yTSUnder,lineLabel,NULL,tsv);
     tsv=lineLabel;
     // Have a default?
     o=(char *)defs->at(i);
     if (o)
        strncpyZ(varsVals+MaxVarValLen*i,o,MaxVarValLen);
    }
 EasyInsertOKCancel(col);
 TDialog *d=col->doItCenter(0);
 delete col;

 if (execDialog(d,varsVals)==cmCancel)
   {
    delete[] varsVals;
    return False;
   }

 return True;
}


