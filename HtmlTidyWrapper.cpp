/*
'
'    HtmlTidyWrapper .NET wrapper for W3C tidy
'    Copyright (C) 2006-2008  Ger Hobbelt (Gerrit E.G. Hobbelt)
'
'    This program is free software: you can redistribute it and/or modify
'    it under the terms of the GNU General Public License as published by
'    the Free Software Foundation, either version 3 of the License, or
'    (at your option) any later version.
'
'    This program is distributed in the hope that it will be useful,
'    but WITHOUT ANY WARRANTY; without even the implied warranty of
'    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'    GNU General Public License for more details.
'
'    You should have received a copy of the GNU General Public License
'    along with this program.  If not, see <http://www.gnu.org/licenses/>.
'
*/


/*
The XmlDocument creation code has been ripped and transformed from the code 
presented by Bjoern Hoehrmann at:

  http://sourceforge.net/mailarchive/forum.php?forum_id=1650&max_rows=25&style=nested&viewmonth=200301

Also refer to

  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vcmex/html/vcconconvertingmanagedextensionsforcprojectsfrompureintermediatelanguagetomixedmode.asp

for information about creating mixed-mode DLLs like this one.

Also see

http://www.developmentnow.com/g/17_2003_8_0_0_90254/-check-commonlanguageruntime-version-link-error.htm

for a way to resolve to chicken-and-egg situation that results from using (or 
not using) nochkclr.obj, as that one is compiled using the wrong run-time 
libraries - at least for our purposes...

*/



using namespace System;
using namespace System::IO;
using namespace System::Xml;
using namespace System::Text;


/*
 * thanks to: 
 *
 *   http://www.thescripts.com/forum/thread281720.html   
 * for a hint how to code an [out] function argument 
 */
using namespace System::Runtime::InteropServices;

/*
   One can override the default behaviour (no forced CG for both
   debug and release builds) by specifying the #define in the
   project properties, so this bit of code will recognize you've
   a mind of your own today.
 */
#ifndef TEST_GC_ACTIVITY
#if defined(NDEBUG) /* release build? NO forced CG, please! */
#define TEST_GC_ACTIVITY 0
#else /* non-release ==> debug build? */
#define TEST_GC_ACTIVITY 0
#endif
#endif


#define VARCONCAT(var1, var2)       VARCONCAT2(var1, var2)
#define VARCONCAT2(var1, var2)      var1 ## var2

#if TEST_GC_ACTIVITY != 0

#define GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(unmanaged_lvalue, managed_rvalue)						    \
			array<unsigned char>^ VARCONCAT(gbc, __LINE__) = managed_rvalue;                                    \
			pin_ptr<unsigned char> VARCONCAT(ngbc, __LINE__) = &VARCONCAT(gbc, __LINE__)[0];                    \
			GC::Collect();                                                                                      \
			GC::WaitForPendingFinalizers();                                                                     \
			ctmbstr unmanaged_lvalue = (ctmbstr)VARCONCAT(ngbc, __LINE__); /* reinterpret_cast<ctmbstr>(ngbc) ? */       

#else

#define GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(unmanaged_lvalue, managed_rvalue)                          \
			array<unsigned char>^ VARCONCAT(gbc, __LINE__) = managed_rvalue;                                    \
			pin_ptr<unsigned char> VARCONCAT(ngbc, __LINE__) = &VARCONCAT(gbc, __LINE__)[0];                    \
			ctmbstr unmanaged_lvalue = (ctmbstr)VARCONCAT(ngbc, __LINE__); /* reinterpret_cast<ctmbstr>(ngbc) ? */       

#endif





#include <errno.h>
#include "tidy.h"
#include "buffio.h"

#include "HtmlTidyWrapper.h"



namespace HtmlTidy 
{



/** Categories of Tidy configuration options
*/
public enum class HtmlTidyConfigCategory
{
  Markup = TidyMarkup,				/**< Markup options: (X)HTML version, etc */
  Diagnostics = TidyDiagnostics,     /**< Diagnostics */
  PrettyPrint = TidyPrettyPrint,     /**< Output layout */
  Encoding = TidyEncoding,			/**< Character encodings */
  Miscellaneous = TidyMiscellaneous    /**< File handling, message format, etc. */
};


/** Option IDs Used to get/set option values.
*/
public enum class HtmlTidyOptionId
{
  UnknownOption = TidyUnknownOption,   /**< Unknown option! */
  IndentSpaces = TidyIndentSpaces,    /**< Indentation n spaces */
  WrapLen = TidyWrapLen,         /**< Wrap margin */
  TabSize = TidyTabSize,         /**< Expand tabs to n spaces */

  CharEncoding = TidyCharEncoding,    /**< In/out character encoding */
  InCharEncoding = TidyInCharEncoding,  /**< Input character encoding (if different) */
  OutCharEncoding = TidyOutCharEncoding, /**< Output character encoding (if different) */    
  Newline = TidyNewline,         /**< Output line ending (default to platform) */

  DoctypeMode = TidyDoctypeMode,     /**< See doctype property */
  Doctype = TidyDoctype,         /**< User specified doctype */

  DuplicateAttrs = TidyDuplicateAttrs,  /**< Keep first or last duplicate attribute */
  AltText = TidyAltText,         /**< Default text for alt attribute */
  
  /* obsolete */
  SlideStyle = TidySlideStyle,      /**< Style sheet for slides: not used for anything yet */

  ErrFile = TidyErrFile,         /**< File name to write errors to */
  OutFile = TidyOutFile,         /**< File name to write markup to */
  WriteBack = TidyWriteBack,       /**< If true then output tidied markup */
  ShowMarkup = TidyShowMarkup,      /**< If false, normal output is suppressed */
  ShowWarnings = TidyShowWarnings,    /**< However errors are always shown */
  Quiet = TidyQuiet,           /**< No 'Parsing X', guessed DTD or summary */
  IndentContent = TidyIndentContent,   /**< Indent content of appropriate tags */
                       /**< "auto" does text/block level content indentation */
  HideEndTags = TidyHideEndTags,     /**< Suppress optional end tags */
  XmlTags = TidyXmlTags,         /**< Treat input as XML */
  XmlOut = TidyXmlOut,          /**< Create output as XML */
  XhtmlOut = TidyXhtmlOut,        /**< Output extensible HTML */
  HtmlOut = TidyHtmlOut,         /**< Output plain HTML, even for XHTML input.
                           Yes means set explicitly. */
  XmlDecl = TidyXmlDecl,         /**< Add <?xml?> for XML docs */
  UpperCaseTags = TidyUpperCaseTags,   /**< Output tags in upper not lower case */
  UpperCaseAttrs = TidyUpperCaseAttrs,  /**< Output attributes in upper not lower case */
  MakeBare = TidyMakeBare,        /**< Make bare HTML: remove Microsoft cruft */
  MakeClean = TidyMakeClean,       /**< Replace presentational clutter by style rules */
  LogicalEmphasis = TidyLogicalEmphasis, /**< Replace i by em and b by strong */
  DropPropAttrs = TidyDropPropAttrs,   /**< Discard proprietary attributes */
  DropFontTags = TidyDropFontTags,    /**< Discard presentation tags */
  DropEmptyParas = TidyDropEmptyParas,  /**< Discard empty p elements */
  DropEmptyOptions = TidyDropEmptyOptions,/**< Discard empty option elements [i_a] */
  FixComments = TidyFixComments,     /**< Fix comments with adjacent hyphens */
  BreakBeforeBR = TidyBreakBeforeBR,   /**< Output newline before <br> or not? */

  /* obsolete */
  BurstSlides = TidyBurstSlides,     /**< Create slides on each h2 element */

  NumEntities = TidyNumEntities,     /**< Use numeric entities */
  QuoteMarks = TidyQuoteMarks,      /**< Output " marks as &quot; */
  QuoteNbsp = TidyQuoteNbsp,       /**< Output non-breaking space as entity */
  QuoteAmpersand = TidyQuoteAmpersand,  /**< Output naked ampersand as &amp; */
  WrapAttVals = TidyWrapAttVals,     /**< Wrap within attribute values */
  WrapScriptlets = TidyWrapScriptlets,  /**< Wrap within JavaScript string literals */
  WrapSection = TidyWrapSection,     /**< Wrap within <![ ... ]> section tags */
  WrapAsp = TidyWrapAsp,         /**< Wrap within ASP pseudo elements */
  WrapJste = TidyWrapJste,        /**< Wrap within JSTE pseudo elements */
  WrapPhp = TidyWrapPhp,         /**< Wrap within PHP pseudo elements */
  FixBackslash = TidyFixBackslash,    /**< Fix URLs by replacing \ with / */
  IndentAttributes = TidyIndentAttributes,/**< Newline+indent before each attribute */
  XmlPIs = TidyXmlPIs,          /**< If set to yes PIs must end with ?> */
  XmlSpace = TidyXmlSpace,        /**< If set to yes adds xml:space attr as needed */
  EncloseBodyText = TidyEncloseBodyText, /**< If yes text at body is wrapped in P's */
  EncloseBlockText = TidyEncloseBlockText,/**< If yes text in blocks is wrapped in P's */
  KeepFileTimes = TidyKeepFileTimes,   /**< If yes last modied time is preserved */
  Word2000 = TidyWord2000,        /**< Draconian cleaning for Word2000 */
  Mark = TidyMark,            /**< Add meta element indicating tidied doc */
  Emacs = TidyEmacs,           /**< If true format error output for GNU Emacs */
  EmacsFile = TidyEmacsFile,       /**< Name of current Emacs file */
  LiteralAttribs = TidyLiteralAttribs,  /**< If true attributes may use newlines */
  BodyOnly = TidyBodyOnly,        /**< Output BODY content only */
  FixUri = TidyFixUri,          /**< Applies URI encoding if necessary */
  LowerLiterals = TidyLowerLiterals,   /**< Folds known attribute values to lower case */
  HideComments = TidyHideComments,    /**< Hides all (real) comments in output */
  IndentCdata = TidyIndentCdata,     /**< Indent <!CDATA[ ... ]]> section */
  ForceOutput = TidyForceOutput,     /**< Output document even if errors were found */
  ShowErrors = TidyShowErrors,      /**< Number of errors to put out */
  AsciiChars = TidyAsciiChars,      /**< Convert quotes and dashes to nearest ASCII char */
  JoinClasses = TidyJoinClasses,     /**< Join multiple class attributes */
  JoinStyles = TidyJoinStyles,      /**< Join multiple style attributes */
  EscapeCdata = TidyEscapeCdata,     /**< Replace <![CDATA[]]> sections with escaped text */

#if SUPPORT_ASIAN_ENCODINGS
  Language = TidyLanguage,        /**< Language property: not used for anything yet */
  NCR = TidyNCR,             /**< Allow numeric character references */
#else
  LanguageNotUsed = TidyLanguageNotUsed,
  NCRNotUsed = TidyNCRNotUsed,
#endif
#if SUPPORT_UTF16_ENCODINGS
  OutputBOM = TidyOutputBOM,      /**< Output a Byte Order Mark (BOM) for UTF-16 encodings */
                      /**< auto: if input stream has BOM, we output a BOM */
#else
  OutputBOMNotUsed = TidyOutputBOMNotUsed,
#endif

  ReplaceColor = TidyReplaceColor,    /**< Replace hex color attribute values with names */
  CSSPrefix = TidyCSSPrefix,       /**< CSS class naming for -clean option */

  InlineTags = TidyInlineTags,      /**< Declared inline tags */
  BlockTags = TidyBlockTags,       /**< Declared block tags */
  EmptyTags = TidyEmptyTags,       /**< Declared empty tags */
  PreTags = TidyPreTags,         /**< Declared pre tags */

  AccessibilityCheckLevel = TidyAccessibilityCheckLevel, /**< Accessibility check level 
                                   0 (old style), or 1, 2, 3 */

  VertSpace = TidyVertSpace,       /**< degree to which markup is spread out vertically */
#if SUPPORT_ASIAN_ENCODINGS
  PunctWrap = TidyPunctWrap,       /**< consider punctuation and breaking spaces for wrapping */
#else
  PunctWrapNotUsed = TidyPunctWrapNotUsed,
#endif
  MergeDivs = TidyMergeDivs,       /**< Merge multiple DIVs */
  DecorateInferredUL = TidyDecorateInferredUL,  /**< Mark inferred UL elements with no indent CSS */
  PreserveEntities = TidyPreserveEntities,    /**< Preserve entities */
  SortAttributes = TidySortAttributes,      /**< Sort attributes */
  MergeSpans = TidyMergeSpans,       /**< Merge multiple SPANs */
  AnchorAsName = TidyAnchorAsName,    /**< Define anchors as name attributes */
  FixTitle = TidyTitle,            /**< Fix an empty title element by filling it with a copy of the first header element in the document body */ 
  N_OPTIONS = N_TIDY_OPTIONS       /**< Must be last */
};

/** Option data types
*/
public enum class HtmlTidyOptionType
{
  String = TidyString,          /**< String */
  Integer = TidyInteger,         /**< Integer or enumeration */
  Boolean = TidyBoolean          /**< Boolean flag */
};


/** AutoBool values used by ParseBool, ParseTriState, ParseIndent, ParseBOM
*/
public enum class HtmlTidyTriState
{
   NoState = TidyNoState,     /**< maps to 'no' */
   YesState = TidyYesState,    /**< maps to 'yes' */
   AutoState = TidyAutoState    /**< Automatic */
};

/** TidyNewline option values to control output line endings.
*/
public enum class HtmlTidyLineEnding
{
    LF = TidyLF,         /**< Use Unix style: LF */
    CRLF = TidyCRLF,       /**< Use DOS/Windows style: CR+LF */
    CR = TidyCR          /**< Use Macintosh style: CR */
};


/** Mode controlling treatment of doctype
*/
public enum class HtmlTidyDoctypeModes
{
    DoctypeOmit = TidyDoctypeOmit,    /**< Omit DOCTYPE altogether */
    DoctypeAuto = TidyDoctypeAuto,    /**< Keep DOCTYPE in input.  Set version to content */
    DoctypeStrict = TidyDoctypeStrict,  /**< Convert document to HTML 4 strict content model */
    DoctypeLoose = TidyDoctypeLoose,   /**< Convert document to HTML 4 transitional content model */
    DoctypeUser = TidyDoctypeUser     /**< Set DOCTYPE FPI explicitly */
};

/** Mode controlling treatment of duplicate Attributes
*/
public enum class HtmlTidyDupAttrModes
{
    KeepFirst = TidyKeepFirst,
    KeepLast = TidyKeepLast
};

/** Mode controlling treatment of sorting attributes
*/
public enum class HtmlTidyAttrSortStrategy
{
    SortAttrNone = TidySortAttrNone,
    SortAttrAlpha = TidySortAttrAlpha
};

/* I/O and Message handling interface
**
** By default, Tidy will define, create and use 
** instances of input and output handlers for 
** standard C buffered I/O (i.e. FILE* stdin,
** FILE* stdout and FILE* stderr for content
** input, content output and diagnostic output,
** respectively.  A FILE* cfgFile input handler
** will be used for config files.  Command line
** options will just be set directly.
*/

/** Message severity level
*/
public enum class HtmlTidyReportLevel
{
  Info = TidyInfo,             /**< Information about markup usage */
  Warning = TidyWarning,          /**< Warning message */
  Config = TidyConfig,           /**< Configuration error */
  Access = TidyAccess,           /**< Accessibility message */
  Error = TidyError,            /**< Error message - output suppressed */
  BadDocument = TidyBadDocument,      /**< I/O or file system error */
  Fatal = TidyFatal             /**< Crash! */
};


/* Document tree traversal functions
*/

/** Node types
*/
public enum class HtmlTidyNodeType
{
  Root = TidyNode_Root,        /**< Root */
  DocType = TidyNode_DocType,     /**< DOCTYPE */
  Comment = TidyNode_Comment,     /**< Comment */
  ProcIns = TidyNode_ProcIns,     /**< Processing Instruction */
  Text = TidyNode_Text,        /**< Text */
  Start = TidyNode_Start,       /**< Start Tag */
  End = TidyNode_End,         /**< End Tag */
  StartEnd = TidyNode_StartEnd,    /**< Start/End (empty) Tag */
  CDATA = TidyNode_CDATA,       /**< Unparsed Text */
  Section = TidyNode_Section,     /**< XML Section */
  Asp = TidyNode_Asp,         /**< ASP Source */
  Jste = TidyNode_Jste,        /**< JSTE Source */
  Php = TidyNode_Php,         /**< PHP Source */
  XmlDecl = TidyNode_XmlDecl      /**< XML Declaration */
};


/** Known HTML element types
*/
public enum class HtmlTidyTagId
{
  UNKNOWN = TidyTag_UNKNOWN,  /**< Unknown tag! */
  A = TidyTag_A,        /**< A */
  ABBR = TidyTag_ABBR,     /**< ABBR */
  ACRONYM = TidyTag_ACRONYM,  /**< ACRONYM */
  ADDRESS = TidyTag_ADDRESS,  /**< ADDRESS */
  ALIGN = TidyTag_ALIGN,    /**< ALIGN */
  APPLET = TidyTag_APPLET,   /**< APPLET */
  AREA = TidyTag_AREA,     /**< AREA */
  B = TidyTag_B,        /**< B */
  BASE = TidyTag_BASE,     /**< BASE */
  BASEFONT = TidyTag_BASEFONT, /**< BASEFONT */
  BDO = TidyTag_BDO,      /**< BDO */
  BGSOUND = TidyTag_BGSOUND,  /**< BGSOUND */
  BIG = TidyTag_BIG,      /**< BIG */
  BLINK = TidyTag_BLINK,    /**< BLINK */
  BLOCKQUOTE = TidyTag_BLOCKQUOTE,   /**< BLOCKQUOTE */
  BODY = TidyTag_BODY,     /**< BODY */
  BR = TidyTag_BR,       /**< BR */
  BUTTON = TidyTag_BUTTON,   /**< BUTTON */
  CAPTION = TidyTag_CAPTION,  /**< CAPTION */
  CENTER = TidyTag_CENTER,   /**< CENTER */
  CITE = TidyTag_CITE,     /**< CITE */
  CODE = TidyTag_CODE,     /**< CODE */
  COL = TidyTag_COL,      /**< COL */
  COLGROUP = TidyTag_COLGROUP, /**< COLGROUP */
  COMMENT = TidyTag_COMMENT,  /**< COMMENT */
  DD = TidyTag_DD,       /**< DD */
  DEL = TidyTag_DEL,      /**< DEL */
  DFN = TidyTag_DFN,      /**< DFN */
  DIR = TidyTag_DIR,      /**< DIR */
  DIV = TidyTag_DIV,      /**< DIF */
  DL = TidyTag_DL,       /**< DL */
  DT = TidyTag_DT,       /**< DT */
  EM = TidyTag_EM,       /**< EM */
  EMBED = TidyTag_EMBED,    /**< EMBED */
  FIELDSET = TidyTag_FIELDSET, /**< FIELDSET */
  FONT = TidyTag_FONT,     /**< FONT */
  FORM = TidyTag_FORM,     /**< FORM */
  FRAME = TidyTag_FRAME,    /**< FRAME */
  FRAMESET = TidyTag_FRAMESET, /**< FRAMESET */
  H1 = TidyTag_H1,       /**< H1 */
  H2 = TidyTag_H2,       /**< H2 */
  H3 = TidyTag_H3,       /**< H3 */
  H4 = TidyTag_H4,       /**< H4 */
  H5 = TidyTag_H5,       /**< H5 */
  H6 = TidyTag_H6,       /**< H6 */
  HEAD = TidyTag_HEAD,     /**< HEAD */
  HR = TidyTag_HR,       /**< HR */
  HTML = TidyTag_HTML,     /**< HTML */
  I = TidyTag_I,        /**< I */
  IFRAME = TidyTag_IFRAME,   /**< IFRAME */
  ILAYER = TidyTag_ILAYER,   /**< ILAYER */
  IMG = TidyTag_IMG,      /**< IMG */
  INPUT = TidyTag_INPUT,    /**< INPUT */
  INS = TidyTag_INS,      /**< INS */
  ISINDEX = TidyTag_ISINDEX,  /**< ISINDEX */
  KBD = TidyTag_KBD,      /**< KBD */
  KEYGEN = TidyTag_KEYGEN,   /**< KEYGEN */
  LABEL = TidyTag_LABEL,    /**< LABEL */
  LAYER = TidyTag_LAYER,    /**< LAYER */
  LEGEND = TidyTag_LEGEND,   /**< LEGEND */
  LI = TidyTag_LI,       /**< LI */
  LINK = TidyTag_LINK,     /**< LINK */
  LISTING = TidyTag_LISTING,  /**< LISTING */
  MAP = TidyTag_MAP,      /**< MAP */
  MARQUEE = TidyTag_MARQUEE,  /**< MARQUEE */
  MENU = TidyTag_MENU,     /**< MENU */
  META = TidyTag_META,     /**< META */
  MULTICOL = TidyTag_MULTICOL, /**< MULTICOL */
  NOBR = TidyTag_NOBR,     /**< NOBR */
  NOEMBED = TidyTag_NOEMBED,  /**< NOEMBED */
  NOFRAMES = TidyTag_NOFRAMES, /**< NOFRAMES */
  NOLAYER = TidyTag_NOLAYER,  /**< NOLAYER */
  NOSAVE = TidyTag_NOSAVE,   /**< NOSAVE */
  NOSCRIPT = TidyTag_NOSCRIPT, /**< NOSCRIPT */
  OBJECT = TidyTag_OBJECT,   /**< OBJECT */
  OL = TidyTag_OL,       /**< OL */
  OPTGROUP = TidyTag_OPTGROUP, /**< OPTGROUP */
  OPTION = TidyTag_OPTION,   /**< OPTION */
  P = TidyTag_P,        /**< P */
  PARAM = TidyTag_PARAM,    /**< PARAM */
  PLAINTEXT = TidyTag_PLAINTEXT,/**< PLAINTEXT */
  PRE = TidyTag_PRE,      /**< PRE */
  Q = TidyTag_Q,        /**< Q */
  RB = TidyTag_RB,       /**< RB */
  RBC = TidyTag_RBC,      /**< RBC */
  RP = TidyTag_RP,       /**< RP */
  RT = TidyTag_RT,       /**< RT */
  RTC = TidyTag_RTC,      /**< RTC */
  RUBY = TidyTag_RUBY,     /**< RUBY */
  S = TidyTag_S,        /**< S */
  SAMP = TidyTag_SAMP,     /**< SAMP */
  SCRIPT = TidyTag_SCRIPT,   /**< SCRIPT */
  SELECT = TidyTag_SELECT,   /**< SELECT */
  SERVER = TidyTag_SERVER,   /**< SERVER */
  SERVLET = TidyTag_SERVLET,  /**< SERVLET */
  SMALL = TidyTag_SMALL,    /**< SMALL */
  SPACER = TidyTag_SPACER,   /**< SPACER */
  SPAN = TidyTag_SPAN,     /**< SPAN */
  STRIKE = TidyTag_STRIKE,   /**< STRIKE */
  STRONG = TidyTag_STRONG,   /**< STRONG */
  STYLE = TidyTag_STYLE,    /**< STYLE */
  SUB = TidyTag_SUB,      /**< SUB */
  SUP = TidyTag_SUP,      /**< SUP */
  TABLE = TidyTag_TABLE,    /**< TABLE */
  TBODY = TidyTag_TBODY,    /**< TBODY */
  TD = TidyTag_TD,       /**< TD */
  TEXTAREA = TidyTag_TEXTAREA, /**< TEXTAREA */
  TFOOT = TidyTag_TFOOT,    /**< TFOOT */
  TH = TidyTag_TH,       /**< TH */
  THEAD = TidyTag_THEAD,    /**< THEAD */
  TITLE = TidyTag_TITLE,    /**< TITLE */
  TR = TidyTag_TR,       /**< TR */
  TT = TidyTag_TT,       /**< TT */
  U = TidyTag_U,        /**< U */
  UL = TidyTag_UL,       /**< UL */
  VAR = TidyTag_VAR,      /**< VAR */
  WBR = TidyTag_WBR,      /**< WBR */
  XMP = TidyTag_XMP,      /**< XMP */
  NEXTID = TidyTag_NEXTID,   /**< NEXTID */

  N_TAGS = N_TIDY_TAGS       /**< Must be last */
};

/* Attribute interrogation
*/

/** Known HTML attributes
*/
public enum class HtmlTidyAttrId
{
  UNKNOWN = TidyAttr_UNKNOWN,           /**< UNKNOWN= */
  ABBR = TidyAttr_ABBR,              /**< ABBR= */
  ACCEPT = TidyAttr_ACCEPT,            /**< ACCEPT= */
  ACCEPT_CHARSET = TidyAttr_ACCEPT_CHARSET,    /**< ACCEPT_CHARSET= */
  ACCESSKEY = TidyAttr_ACCESSKEY,         /**< ACCESSKEY= */
  ACTION = TidyAttr_ACTION,            /**< ACTION= */
  ADD_DATE = TidyAttr_ADD_DATE,          /**< ADD_DATE= */
  ALIGN = TidyAttr_ALIGN,             /**< ALIGN= */
  ALINK = TidyAttr_ALINK,             /**< ALINK= */
  ALT = TidyAttr_ALT,               /**< ALT= */
  ARCHIVE = TidyAttr_ARCHIVE,           /**< ARCHIVE= */
  AXIS = TidyAttr_AXIS,              /**< AXIS= */
  BACKGROUND = TidyAttr_BACKGROUND,        /**< BACKGROUND= */
  BGCOLOR = TidyAttr_BGCOLOR,           /**< BGCOLOR= */
  BGPROPERTIES = TidyAttr_BGPROPERTIES,      /**< BGPROPERTIES= */
  BORDER = TidyAttr_BORDER,            /**< BORDER= */
  BORDERCOLOR = TidyAttr_BORDERCOLOR,       /**< BORDERCOLOR= */
  BOTTOMMARGIN = TidyAttr_BOTTOMMARGIN,      /**< BOTTOMMARGIN= */
  CELLPADDING = TidyAttr_CELLPADDING,       /**< CELLPADDING= */
  CELLSPACING = TidyAttr_CELLSPACING,       /**< CELLSPACING= */
  CHAR = TidyAttr_CHAR,              /**< CHAR= */
  CHAROFF = TidyAttr_CHAROFF,           /**< CHAROFF= */
  CHARSET = TidyAttr_CHARSET,           /**< CHARSET= */
  CHECKED = TidyAttr_CHECKED,           /**< CHECKED= */
  CITE = TidyAttr_CITE,              /**< CITE= */
  CLASS = TidyAttr_CLASS,             /**< CLASS= */
  CLASSID = TidyAttr_CLASSID,           /**< CLASSID= */
  CLEAR = TidyAttr_CLEAR,             /**< CLEAR= */
  CODE = TidyAttr_CODE,              /**< CODE= */
  CODEBASE = TidyAttr_CODEBASE,          /**< CODEBASE= */
  CODETYPE = TidyAttr_CODETYPE,          /**< CODETYPE= */
  COLOR = TidyAttr_COLOR,             /**< COLOR= */
  COLS = TidyAttr_COLS,              /**< COLS= */
  COLSPAN = TidyAttr_COLSPAN,           /**< COLSPAN= */
  COMPACT = TidyAttr_COMPACT,           /**< COMPACT= */
  CONTENT = TidyAttr_CONTENT,           /**< CONTENT= */
  COORDS = TidyAttr_COORDS,            /**< COORDS= */
  DATA = TidyAttr_DATA,              /**< DATA= */
  DATAFLD = TidyAttr_DATAFLD,           /**< DATAFLD= */
  DATAFORMATAS = TidyAttr_DATAFORMATAS,      /**< DATAFORMATAS= */
  DATAPAGESIZE = TidyAttr_DATAPAGESIZE,      /**< DATAPAGESIZE= */
  DATASRC = TidyAttr_DATASRC,           /**< DATASRC= */
  DATETIME = TidyAttr_DATETIME,          /**< DATETIME= */
  DECLARE = TidyAttr_DECLARE,           /**< DECLARE= */
  DEFER = TidyAttr_DEFER,             /**< DEFER= */
  DIR = TidyAttr_DIR,               /**< DIR= */
  DISABLED = TidyAttr_DISABLED,          /**< DISABLED= */
  ENCODING = TidyAttr_ENCODING,          /**< ENCODING= */
  ENCTYPE = TidyAttr_ENCTYPE,           /**< ENCTYPE= */
  FACE = TidyAttr_FACE,              /**< FACE= */
  FOR = TidyAttr_FOR,               /**< FOR= */
  FRAME = TidyAttr_FRAME,             /**< FRAME= */
  FRAMEBORDER = TidyAttr_FRAMEBORDER,       /**< FRAMEBORDER= */
  FRAMESPACING = TidyAttr_FRAMESPACING,      /**< FRAMESPACING= */
  GRIDX = TidyAttr_GRIDX,             /**< GRIDX= */
  GRIDY = TidyAttr_GRIDY,             /**< GRIDY= */
  HEADERS = TidyAttr_HEADERS,           /**< HEADERS= */
  HEIGHT = TidyAttr_HEIGHT,            /**< HEIGHT= */
  HREF = TidyAttr_HREF,              /**< HREF= */
  HREFLANG = TidyAttr_HREFLANG,          /**< HREFLANG= */
  HSPACE = TidyAttr_HSPACE,            /**< HSPACE= */
  HTTP_EQUIV = TidyAttr_HTTP_EQUIV,        /**< HTTP_EQUIV= */
  ID = TidyAttr_ID,                /**< ID= */
  ISMAP = TidyAttr_ISMAP,             /**< ISMAP= */
  LABEL = TidyAttr_LABEL,             /**< LABEL= */
  LANG = TidyAttr_LANG,              /**< LANG= */
  LANGUAGE = TidyAttr_LANGUAGE,          /**< LANGUAGE= */
  LAST_MODIFIED = TidyAttr_LAST_MODIFIED,     /**< LAST_MODIFIED= */
  LAST_VISIT = TidyAttr_LAST_VISIT,        /**< LAST_VISIT= */
  LEFTMARGIN = TidyAttr_LEFTMARGIN,        /**< LEFTMARGIN= */
  LINK = TidyAttr_LINK,              /**< LINK= */
  LONGDESC = TidyAttr_LONGDESC,          /**< LONGDESC= */
  LOWSRC = TidyAttr_LOWSRC,            /**< LOWSRC= */
  MARGINHEIGHT = TidyAttr_MARGINHEIGHT,      /**< MARGINHEIGHT= */
  MARGINWIDTH = TidyAttr_MARGINWIDTH,       /**< MARGINWIDTH= */
  MAXLENGTH = TidyAttr_MAXLENGTH,         /**< MAXLENGTH= */
  MEDIA = TidyAttr_MEDIA,             /**< MEDIA= */
  METHOD = TidyAttr_METHOD,            /**< METHOD= */
  MULTIPLE = TidyAttr_MULTIPLE,          /**< MULTIPLE= */
  NAME = TidyAttr_NAME,              /**< NAME= */
  NOHREF = TidyAttr_NOHREF,            /**< NOHREF= */
  NORESIZE = TidyAttr_NORESIZE,          /**< NORESIZE= */
  NOSHADE = TidyAttr_NOSHADE,           /**< NOSHADE= */
  NOWRAP = TidyAttr_NOWRAP,            /**< NOWRAP= */
  OBJECT = TidyAttr_OBJECT,            /**< OBJECT= */
  OnAFTERUPDATE = TidyAttr_OnAFTERUPDATE,     /**< OnAFTERUPDATE= */
  OnBEFOREUNLOAD = TidyAttr_OnBEFOREUNLOAD,    /**< OnBEFOREUNLOAD= */
  OnBEFOREUPDATE = TidyAttr_OnBEFOREUPDATE,    /**< OnBEFOREUPDATE= */
  OnBLUR = TidyAttr_OnBLUR,            /**< OnBLUR= */
  OnCHANGE = TidyAttr_OnCHANGE,          /**< OnCHANGE= */
  OnCLICK = TidyAttr_OnCLICK,           /**< OnCLICK= */
  OnDATAAVAILABLE = TidyAttr_OnDATAAVAILABLE,   /**< OnDATAAVAILABLE= */
  OnDATASETCHANGED = TidyAttr_OnDATASETCHANGED,  /**< OnDATASETCHANGED= */
  OnDATASETCOMPLETE = TidyAttr_OnDATASETCOMPLETE, /**< OnDATASETCOMPLETE= */
  OnDBLCLICK = TidyAttr_OnDBLCLICK,        /**< OnDBLCLICK= */
  OnERRORUPDATE = TidyAttr_OnERRORUPDATE,     /**< OnERRORUPDATE= */
  OnFOCUS = TidyAttr_OnFOCUS,           /**< OnFOCUS= */
  OnKEYDOWN = TidyAttr_OnKEYDOWN,         /**< OnKEYDOWN= */
  OnKEYPRESS = TidyAttr_OnKEYPRESS,        /**< OnKEYPRESS= */
  OnKEYUP = TidyAttr_OnKEYUP,           /**< OnKEYUP= */
  OnLOAD = TidyAttr_OnLOAD,            /**< OnLOAD= */
  OnMOUSEDOWN = TidyAttr_OnMOUSEDOWN,       /**< OnMOUSEDOWN= */
  OnMOUSEMOVE = TidyAttr_OnMOUSEMOVE,       /**< OnMOUSEMOVE= */
  OnMOUSEOUT = TidyAttr_OnMOUSEOUT,        /**< OnMOUSEOUT= */
  OnMOUSEOVER = TidyAttr_OnMOUSEOVER,       /**< OnMOUSEOVER= */
  OnMOUSEUP = TidyAttr_OnMOUSEUP,         /**< OnMOUSEUP= */
  OnRESET = TidyAttr_OnRESET,           /**< OnRESET= */
  OnROWENTER = TidyAttr_OnROWENTER,        /**< OnROWENTER= */
  OnROWEXIT = TidyAttr_OnROWEXIT,         /**< OnROWEXIT= */
  OnSELECT = TidyAttr_OnSELECT,          /**< OnSELECT= */
  OnSUBMIT = TidyAttr_OnSUBMIT,          /**< OnSUBMIT= */
  OnUNLOAD = TidyAttr_OnUNLOAD,          /**< OnUNLOAD= */
  PROFILE = TidyAttr_PROFILE,           /**< PROFILE= */
  PROMPT = TidyAttr_PROMPT,            /**< PROMPT= */
  RBSPAN = TidyAttr_RBSPAN,            /**< RBSPAN= */
  READONLY = TidyAttr_READONLY,          /**< READONLY= */
  REL = TidyAttr_REL,               /**< REL= */
  REV = TidyAttr_REV,               /**< REV= */
  RIGHTMARGIN = TidyAttr_RIGHTMARGIN,       /**< RIGHTMARGIN= */
  ROWS = TidyAttr_ROWS,              /**< ROWS= */
  ROWSPAN = TidyAttr_ROWSPAN,           /**< ROWSPAN= */
  RULES = TidyAttr_RULES,             /**< RULES= */
  SCHEME = TidyAttr_SCHEME,            /**< SCHEME= */
  SCOPE = TidyAttr_SCOPE,             /**< SCOPE= */
  SCROLLING = TidyAttr_SCROLLING,         /**< SCROLLING= */
  SELECTED = TidyAttr_SELECTED,          /**< SELECTED= */
  SHAPE = TidyAttr_SHAPE,             /**< SHAPE= */
  SHOWGRID = TidyAttr_SHOWGRID,          /**< SHOWGRID= */
  SHOWGRIDX = TidyAttr_SHOWGRIDX,         /**< SHOWGRIDX= */
  SHOWGRIDY = TidyAttr_SHOWGRIDY,         /**< SHOWGRIDY= */
  SIZE = TidyAttr_SIZE,              /**< SIZE= */
  SPAN = TidyAttr_SPAN,              /**< SPAN= */
  SRC = TidyAttr_SRC,               /**< SRC= */
  STANDBY = TidyAttr_STANDBY,           /**< STANDBY= */
  START = TidyAttr_START,             /**< START= */
  STYLE = TidyAttr_STYLE,             /**< STYLE= */
  SUMMARY = TidyAttr_SUMMARY,           /**< SUMMARY= */
  TABINDEX = TidyAttr_TABINDEX,          /**< TABINDEX= */
  TARGET = TidyAttr_TARGET,            /**< TARGET= */
  TEXT = TidyAttr_TEXT,              /**< TEXT= */
  TITLE = TidyAttr_TITLE,             /**< TITLE= */
  TOPMARGIN = TidyAttr_TOPMARGIN,         /**< TOPMARGIN= */
  TYPE = TidyAttr_TYPE,              /**< TYPE= */
  USEMAP = TidyAttr_USEMAP,            /**< USEMAP= */
  VALIGN = TidyAttr_VALIGN,            /**< VALIGN= */
  VALUE = TidyAttr_VALUE,             /**< VALUE= */
  VALUETYPE = TidyAttr_VALUETYPE,         /**< VALUETYPE= */
  VERSION = TidyAttr_VERSION,           /**< VERSION= */
  VLINK = TidyAttr_VLINK,             /**< VLINK= */
  VSPACE = TidyAttr_VSPACE,            /**< VSPACE= */
  WIDTH = TidyAttr_WIDTH,             /**< WIDTH= */
  WRAP = TidyAttr_WRAP,              /**< WRAP= */
  XML_LANG = TidyAttr_XML_LANG,          /**< XML_LANG= */
  XML_SPACE = TidyAttr_XML_SPACE,         /**< XML_SPACE= */
  XMLNS = TidyAttr_XMLNS,             /**< XMLNS= */

  EVENT = TidyAttr_EVENT,             /**< EVENT= */
  METHODS = TidyAttr_METHODS,           /**< METHODS= */
  N = TidyAttr_N,                 /**< N= */
  SDAFORM = TidyAttr_SDAFORM,           /**< SDAFORM= */
  SDAPREF = TidyAttr_SDAPREF,           /**< SDAPREF= */
  SDASUFF = TidyAttr_SDASUFF,           /**< SDASUFF= */
  URN = TidyAttr_URN,               /**< URN= */

  N_ATTRIBS = N_TIDY_ATTRIBS              /**< Must be last */
};


/* [i_a] */
/** Block-level and unknown elements are printed on
  new lines and their contents indented 2 spaces.

  Inline elements are printed inline.

  Inline content is wrapped on spaces (except in
  attribute values or preformatted text, after
  start tags and before end tags.
*/
public enum class HtmlTidyTextFormat
{
  Normal = TidyTextFormat_Normal,	/**< NORMAL (default) */ 
  Preformatted = TidyTextFormat_Preformatted,	/**< PREFORMATTED */
  Comment = TidyTextFormat_Comment,	/**< COMMENT */
  AttribValue = TidyTextFormat_AttribValue,	/**< ATTRIBVALUE */
  NoWrap = TidyTextFormat_NoWrap,	/**< NOWRAP */
  CDATA = TidyTextFormat_CDATA,	/**< CDATA */
}; 
/* Warning: TidyTextFormat values must match the #define's in pprint.h exactly! */







#pragma unmanaged

	static uint tmbstrlen(ctmbstr str)
	{
		uint len = 0;
		while(*str++)
		{
			++len;
		}
		return len;
	}

#pragma managed



	/*
	 * Dummy UrlResolver added to prevent the XmlDocument from accessing the internet (and throwing exceptions)
	 * while trying to validate the DOCTYPE produced by HtmlTidy.
	 *
	 * See
	 *   http://groups.google.com/group/microsoft.public.dotnet.xml/browse_thread/thread/be405f41e4e158e4/122a8e219c7a7d17?lnk=st&q=xmldocument+load+GetEntity&rnum=3&hl=en#122a8e219c7a7d17
	 * for more info.
	 */
	private ref class DummyUrlResolver: public XmlUrlResolver 
	{
	public:
		virtual Object^ GetEntity(Uri^ absoluteUri, String^ role, Type^ ofObjectToReturn) override 
		{
			if (ofObjectToReturn == nullptr) 
			{
				return Stream::Null;
			}
			if (ofObjectToReturn != nullptr) 
			{
				return Stream::Null;
			}
			return XmlUrlResolver::GetEntity(absoluteUri, role, ofObjectToReturn);
		}
	};




	public ref class TidyParser: public XmlTextReader
	{
	protected:
		TidyDoc tdoc;
		TidyBuffer* errbuf;
		TidyOutputSink *errsink;
		int rc;


		void CopyAllAttributes(XmlNode^ e, XmlDocument^ doc, TidyNode n)
		{
			TidyAttr att;
			for (att = tidyAttrFirst(n); att; att = tidyAttrNext(att))
			{
				ctmbstr tattname = tidyAttrName(att);
				ctmbstr tattval = tidyAttrValue(att);
				String^ attname = nullptr;
				String^ attval = nullptr;
				if (tattname != NULL)
				{
					attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
				}
				if (tattval != NULL)
				{
					attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
				}

				/*
				* Attribute Attribute Interrogation
				*
				* Get information about any given attribute.
				*/

				TidyAttrId id = tidyAttrGetId(att);
				Bool AttrIsEvent = tidyAttrIsEvent(att);
				Bool AttrIsProp = tidyAttrIsProp(att);

				switch (id)
				{
				case TidyAttr_XMLNS:
					break;

				default:
					{
				XmlAttribute^ a = doc->CreateAttribute(attname);
				if (attval != nullptr)
				{
					a->Value = attval;
				}
				e->Attributes->Append(a);
					}
					break;
				}
			}
		}



		String^ GetRawTextInNode(TidyNode n, TidyTextFormat mode)
		{
			String^ textstr = String::Empty;
			Bool hastext = tidyNodeHasText(tdoc, n);
			if (hastext)
			{
				TidyBuffer textbuf = {0};
				Bool gettxt = tidyNodeGetRawText(tdoc, n, mode, 0, &textbuf);
				if (gettxt && textbuf.bp)
				{
					textstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
				}
				tidyBufFree(&textbuf);
			}

			return textstr;
		}

		String^ GetFormattedTextInNode(TidyNode n, TidyTextFormat mode)
		{
			String^ textstr = String::Empty;
			Bool hastext = tidyNodeHasText(tdoc, n);
			if (hastext)
			{
				TidyBuffer textbuf = {0};
				Bool gettxt = tidyNodeGetFormattedText(tdoc, n, mode, 0, &textbuf);
				if (gettxt && textbuf.bp)
				{
					textstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
				}
				tidyBufFree(&textbuf);
			}

			return textstr;
		}




		void process_tree(TidyNode node, XmlDocument^ doc, XmlNode^ current)
		{
			TidyNode n = node;

			if (!doc || !node)
				return;

			if (!current)
				current = doc;

			while (n)
			{
				TidyNodeType t = tidyNodeGetType(n);
				TidyNode child = tidyGetChild(n);
				TidyTagId tagid = tidyNodeGetId(n);

				ctmbstr nodename = tidyNodeGetName(n);

				{
					Bool istext = tidyNodeIsText(n);
					Bool isprop = tidyNodeIsProp(tdoc, n);
					Bool isheader = tidyNodeIsHeader(n);
					Bool hastext = tidyNodeHasText(tdoc, n);
					TidyBuffer textbuf = {0};
					Bool gettxt = tidyNodeGetText(tdoc, n, &textbuf);
					String^ textstr = String::Empty;
					if (gettxt && textbuf.bp)
					{
						textstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
					}
					tidyBufFree(&textbuf);
					Bool getcmttxt = tidyNodeGetFormattedText(tdoc, n, TidyTextFormat_Comment, 0, &textbuf);
					String^ cmttextstr = String::Empty;
					if (getcmttxt && textbuf.bp)
					{
						cmttextstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
					}
					tidyBufFree(&textbuf);
					Bool getattrtxt = tidyNodeGetFormattedText(tdoc, n, TidyTextFormat_AttribValue, 0, &textbuf);
					String^ attrtextstr = String::Empty;
					if (getattrtxt && textbuf.bp)
					{
						attrtextstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
					}
					tidyBufFree(&textbuf);
					Bool getrawtxt = tidyNodeGetRawText(tdoc, n, TidyTextFormat_Normal, 0, &textbuf);
					String^ rawtextstr = String::Empty;
					if (getrawtxt && textbuf.bp)
					{
						rawtextstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
					}
					tidyBufFree(&textbuf);
				// Copy the unescaped value of this node into the given TidyBuffer as UTF-8 
					Bool getnodeValueTxt = tidyNodeGetValue(tdoc, n, &textbuf);
					String^ nodeValuetextstr = String::Empty;
					if (getnodeValueTxt && textbuf.bp)
					{
						nodeValuetextstr = gcnew String((ctmbstr)textbuf.bp, 0, textbuf.size, Text::Encoding::UTF8);
					}
					tidyBufFree(&textbuf);

					uint line = tidyNodeLine(n);
					uint column = tidyNodeColumn(n);
				}


				switch (t)
				{
				case TidyNode_Root:        /* Root */
					process_tree(child, doc, current);
					break;

				case TidyNode_Start:       /* Start Tag */
					{
						ctmbstr tname = tidyNodeGetName(n);
						String^ name = nullptr;
						if (tname != NULL)
						{
							XmlNode^ e = nullptr;
							
							name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

							switch (tagid)
							{
							case TidyTag_HTML:
								// look up the 'xmlns' namespace attribute; see if it exists and if it does, fetch it.
								{
								TidyAttr xmlns_attr = tidyAttrGetById(n, TidyAttr_XMLNS);

								if (xmlns_attr)
								{
									ctmbstr tattname = tidyAttrName(xmlns_attr);
									ctmbstr tattval = tidyAttrValue(xmlns_attr);
									String^ attname = nullptr;
									String^ attval = nullptr;
									if (tattname != NULL)
									{
										attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
									}
									if (tattval != NULL)
									{
										attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
									}
							
									e = doc->CreateElement(name, attval);
								}
								}
								break;

							default:
								break;
							}

							if (e == nullptr)
							{
								e = doc->CreateElement(name);
							}

							CopyAllAttributes(e, doc, n);

							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						else
						{
							throw gcnew Exception(gcnew String("Tidy START Node has no name"));
						}
						break;
					}

				case TidyNode_DocType:     /* DOCTYPE */
					{
						ctmbstr tname = "DOCTYPE";                 
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);
						ctmbstr nodename = tidyNodeGetName(n);
						String^ nodestr = nullptr;
						if (nodename)
						{
							nodestr = gcnew String(nodename, 0, tmbstrlen(nodename), Text::Encoding::UTF8);
						}
						if (nodestr == nullptr)
						{
							nodestr = name;
						}
						TidyAttr fpi = tidyGetAttrByName(n, "PUBLIC" );
						TidyAttr sys = tidyGetAttrByName(n, "SYSTEM" );
						String^ pubatt = nullptr;
						String^ sysatt = nullptr;
						if (fpi)
						{
							ctmbstr tattname = tidyAttrName(fpi);
							ctmbstr tattval = tidyAttrValue(fpi);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
							}
							pubatt = attval;
						}
						if (sys)
						{
							ctmbstr tattname = tidyAttrName(sys);
							ctmbstr tattval = tidyAttrValue(sys);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
							}
							sysatt = attval;
						}

						XmlDocumentType^ e = nullptr;
						try
						{
							e = doc->CreateDocumentType(nodestr, pubatt, sysatt, nullptr);
						}
						catch (Exception^ ex)
						{
							// CreateDocumentType may try to access the internet :-(((
							// and it'll throw an exception if it can't 'validate' those pub and sys items in there...
							e = nullptr;
						}

						if (e != nullptr)
						{
							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						break;
					}
				case TidyNode_Comment:     /* Comment */
					{
						ctmbstr tname = "Comment";                 
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						String^ textstr = GetRawTextInNode(n, TidyTextFormat_Normal);

						XmlComment^ e = doc->CreateComment(textstr);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_ProcIns:     /* Processing Instruction */
					{
						ctmbstr tname = "Processing Instruction";
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						XmlNode^ e = doc->CreateProcessingInstruction(name, String::Empty);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Text:        /* Text */
					{
						ctmbstr tname = "Text";                    
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						String^ textstr = GetRawTextInNode(n, TidyTextFormat_Normal);

						XmlNode^ e = doc->CreateTextNode(textstr);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_CDATA:       /* Unparsed Text */
					{
						ctmbstr tname = "CDATA";                   
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						String^ textstr = GetRawTextInNode(n, TidyTextFormat_Normal);

						XmlNode^ e = doc->CreateCDataSection(textstr);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Section:     /* XML Section */
					{
						ctmbstr tname = "XML Section";             
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						XmlNode^ e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Asp:         /* ASP Source */
					{
						ctmbstr tname = "ASP";                     
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						XmlNode^ e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Jste:        /* JSTE Source */
					{
						ctmbstr tname = "JSTE";                    
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						XmlNode^ e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Php:         /* PHP Source */
					{
						ctmbstr tname = "PHP";                     
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

						XmlNode^ e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_XmlDecl:     /* XML Declaration */
					{
						ctmbstr tname = "XML Declaration";         
						String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);
						TidyAttr ver = tidyAttrGetById(n, TidyAttr_VERSION);
						TidyAttr enc = tidyAttrGetById(n, TidyAttr_ENCODING);
						TidyAttr sa = tidyGetAttrByName(n, "standalone");
						String^ veratt = String::Empty;
						String^ encatt = String::Empty;
						String^ saatt = String::Empty;
						if (ver)
						{
							ctmbstr tattname = tidyAttrName(ver);
							ctmbstr tattval = tidyAttrValue(ver);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
							}
							veratt = attval;
						}
						if (enc)
						{
							ctmbstr tattname = tidyAttrName(enc);
							ctmbstr tattval = tidyAttrValue(enc);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
							}
							encatt = attval;
						}
						if (sa)
						{
							ctmbstr tattname = tidyAttrName(sa);
							ctmbstr tattval = tidyAttrValue(sa);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Text::Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Text::Encoding::UTF8);
							}
							saatt = attval;
						}

						XmlNode^ e = doc->CreateXmlDeclaration(veratt, encatt, saatt);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}

				case TidyNode_End:         /* End Tag */
					{
						ctmbstr tname = tidyNodeGetName(n);
						if (tname)
						{
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

							XmlNode^ e = doc->CreateElement(name);

							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						else
						{
							throw gcnew Exception(gcnew String("Tidy END Node has no name"));
						}
						break;
					}
				case TidyNode_StartEnd:    /* Start/End (empty) Tag */
					{
						ctmbstr tname = tidyNodeGetName(n);
						if (tname)
						{
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

							XmlNode^ e = doc->CreateElement(name);

							CopyAllAttributes(e, doc, n);

							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						else
						{
							throw gcnew Exception(gcnew String("Tidy START+END Node has no name"));
						}
						break;
					}
				default:
					{
						ctmbstr tname = tidyNodeGetName(n);
						if (tname)
						{
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Text::Encoding::UTF8);

							XmlNode^ e = doc->CreateElement(name);

							CopyAllAttributes(e, doc, n);

							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						else
						{
							throw gcnew Exception(String::Format("Tidy Node type {0} has no name", (int)t));
						}
						break;
					}
				}

				n = tidyGetNext(n);
			}
		}


		int RunHtmlTidyProcess(String^ src)
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(input, Text::Encoding::UTF8->GetBytes(src));

			int status = tidyParseString(tdoc, input);
			if (status >= 0)
			{
				status = tidyCleanAndRepair(tdoc);
			}
			if (status >= 0)
			{
				status = tidyRunDiagnostics(tdoc);
			}

			return status;
		}

		int InitTidyParser(void)
		{
			rc = 0;
			errsink = 0;
			errbuf = 0;

			tdoc = tidyCreate();
			if (tdoc)
			{
				errbuf = tidyBufCreate(NULL);
				if (errbuf)
				{
					tidyBufAlloc(errbuf, 8192);  // 8K error message buffer
				}

				errsink = tidyOutputSinkCreate(tdoc);
				if (errsink)
				{
					tidyInitOutputBuffer(errsink, errbuf);

					rc = tidySetErrorSink(tdoc, errsink);   // Capture diagnostics
				}
				
				if (rc < 0 || !errsink || !errbuf)
				{
					if (errsink)
					{
						tidyOutputSinkDestroy(tdoc, errsink);
						errsink = 0;
					}
					if (tdoc != 0)
					{
						tidyRelease(tdoc);
						tdoc = 0;
					}
					if (errbuf != 0)
					{
						tidyBufDestroy(errbuf);
						errbuf = 0;
					}
				}
			}
			else
			{
				rc = -EINVAL;
			}
			return rc;
		};


	public:
		TidyParser(): XmlTextReader()
		{
			InitTidyParser();
		};

		TidyParser(String^ src): XmlTextReader()
		{
			int status = InitTidyParser();
			if (status >= 0)
			{
				status = RunHtmlTidyProcess(src);
			}
		};

		~TidyParser()
		{
			if (errsink != 0)
			{
				tidyOutputSinkDestroy(tdoc, errsink);
				errsink = 0;
			}
			if (tdoc != 0)
			{
				tidyRelease(tdoc);
				tdoc = 0;
			}
			if (errbuf != 0)
			{
				tidyBufDestroy(errbuf);
				errbuf = 0;
			}
		};

		virtual void Close() override
		{
			XmlTextReader::Close();

			if (errsink != 0)
			{
				tidyOutputSinkDestroy(tdoc, errsink);
				errsink = 0;
			}
			if (tdoc != 0)
			{
				tidyRelease(tdoc);
				tdoc = 0;
			}
			if (errbuf != 0)
			{
				// tidyBufFree(errbuf);
				tidyBufDestroy(errbuf);
				errbuf = 0;
			}
		};


		String^ ErrorMessage(void)
		{
			if (errbuf)
			{
				String^ ret = gcnew String((ctmbstr)errbuf->bp, 0, errbuf->size, Text::Encoding::UTF8);

				return ret;
			}
			else if (rc < 0)
			{
				String^ ret = gcnew String("HtmlTidyWrapper initialization failure");

				return ret;
			}
			else
			{
				String^ ret = String::Empty;

				return ret;
			}
		};

		void ResetErrorMessage(void)
		{
			if (errbuf)
			{
				tidyBufClear(errbuf);
			}
		};



		XmlDocument^ DoParseHtml(String^ s)
		{
							int status = RunHtmlTidyProcess(s);

							if (status >= 0)
			{
				TidyNode root = tidyGetRoot(tdoc);

				XmlDocument^ doc = gcnew XmlDocument();
				doc->XmlResolver = gcnew DummyUrlResolver();
				XmlNode^ node;
				process_tree(root, doc, node);

				return doc;
			}
			return nullptr;
		};

		String^ DoParseHtml2String(String^ s)
		{
			int status = RunHtmlTidyProcess(s);

			String^ result = nullptr;

			if (status >= 0)
			{
				TidyBuffer outbuf = {0};
				TidyOutputSink outsink = {0};
				tidyInitOutputBuffer(&outsink, &outbuf);
				status = tidySaveSink(tdoc, &outsink);
				if (status >= 0)
				{
					result = gcnew String((ctmbstr)outbuf.bp, 0, outbuf.size, Text::Encoding::UTF8);
				}
				tidyBufFree(&outbuf);
			}
			return result;
		};


		/** Get release date (version) for current library */
		String^ TidyReleaseDate(void)
		{
			ctmbstr str = tidyReleaseDate();
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);

			return ret;
		};

		/* Diagnostics and Repair
		*/

		/** Get status of current document. */
		int TidyStatus(void)
		{
			return tidyStatus(tdoc);
		};

		/** Detected HTML version: 0, 2, 3 or 4 */
		int TidyDetectedHtmlVersion(void) 
		{
			return tidyDetectedHtmlVersion(tdoc);
		};

		/** Input is XHTML? */
		bool TidyDetectedXhtml(void) 
		{
			return (no != tidyDetectedXhtml(tdoc));
		};

		/** Input is generic XML (not HTML or XHTML)? */
		bool TidyDetectedGenericXml(void) 
		{
			return (no != tidyDetectedGenericXml(tdoc));
		};

		/** Number of Tidy errors encountered. If > 0, output is suppressed
		** unless TidyForceOutput is set.
		*/
		uint TidyErrorCount(void) 
		{
			return tidyErrorCount(tdoc);
		};

		/** Number of Tidy warnings encountered. */
		uint TidyWarningCount(void) 
		{
			return tidyWarningCount(tdoc);
		};

		/** Number of Tidy accessibility warnings encountered. */
		uint TidyAccessWarningCount(void) 
		{
			return tidyAccessWarningCount(tdoc);
		};

		/** Number of Tidy configuration errors encountered. */
		uint TidyConfigErrorCount(void) 
		{
			return tidyConfigErrorCount(tdoc);
		};

		/* Get/Set configuration options
		*/
		/** Load an ASCII Tidy configuration, stored in the config String^ */
		int TidyLoadConfigFromString(String^ config) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Text::Encoding::UTF8->GetBytes(config));
			TidyBuffer buffer = {0};
			tidyBufAttach(&buffer, (byte *)str, tmbstrlen(str) + 1);

			int ret = tidyLoadConfigBuffer(tdoc, &buffer);
			tidyBufDetach(&buffer);

			return ret;
		};

		int TidyLoadConfigFromFile(String^ configfile) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Text::Encoding::UTF8->GetBytes(configfile));
			return tidyLoadConfig(tdoc, fname);
		};

		/** Load a Tidy configuration, stored in the config string, with the specified character encoding */
		int TidyLoadConfigFromStringEnc(String^ config, String^ charenc) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Text::Encoding::UTF8->GetBytes(config));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Text::Encoding::UTF8->GetBytes(charenc));
			TidyBuffer buffer = {0};
			tidyBufAttach(&buffer, (byte *)str, tmbstrlen(str) + 1);

			int ret = tidyLoadConfigBufferEnc(tdoc, &buffer, enc);
			tidyBufDetach(&buffer);

			return ret;
		};

		int TidyLoadConfigEncFromFile(String^ configfile, String^ charenc) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Text::Encoding::UTF8->GetBytes(configfile));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Text::Encoding::UTF8->GetBytes(charenc));
			return tidyLoadConfigEnc(tdoc, fname, enc);
		};



		/** Set the input/output character encoding for parsing markup.
		** Values include: ascii, latin1, raw, utf8, iso2022, mac,
		** win1252, utf16le, utf16be, utf16, big5 and shiftjis. Case in-sensitive.
		*/
		int TidySetCharEncoding(String^ encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Text::Encoding::UTF8->GetBytes(encnam));
			return tidySetCharEncoding(tdoc, enc);
		};

		/** Set the input encoding for parsing markup.
		** As for TidySetCharEncoding but only affects the input encoding
		**/
		int TidySetInCharEncoding(String^ encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Text::Encoding::UTF8->GetBytes(encnam));
			return tidySetInCharEncoding(tdoc, enc);
		};

		/** Set the output encoding.
		**/
		int TidySetOutCharEncoding(String^ encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Text::Encoding::UTF8->GetBytes(encnam));
			return tidySetOutCharEncoding(tdoc, enc);
		};

		/** @} end Basic group */


		/** @defgroup Configuration Configuration Options
		**
		** Functions for getting and setting Tidy configuration options.
		** @{
		*/

		/** Applications using TidyLib may want to augment command-line and
		** configuration file options. Setting this callback allows an application 
		** developer to examine command-line and configuration file options after
		** TidyLib has examined them and failed to recognize them.
		**/




		/** Get option ID by name */
		HtmlTidyOptionId TidyOptGetIdForName(String^ optnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(name, Text::Encoding::UTF8->GetBytes(optnam));
			return (HtmlTidyOptionId)tidyOptGetIdForName(name);
		};

		/** Get iterator for list of option */
		/** 
		Example:
		<pre>
		TidyIterator itOpt = TidyGetOptionList(tdoc) {};
		while (itOpt)
		{
		TidyOption opt = TidyGetNextOption(tdoc, &itOpt) {};
		.. get/set option values ..
		}
		</pre>
		*/

		TidyIterator TidyGetOptionList(void) 
		{
			return tidyGetOptionList(tdoc);
		};
		/** Get next Option */
		TidyOption TidyGetNextOption(TidyIterator* pos) 
		{
			return tidyGetNextOption(tdoc, pos);
		};

		/** Lookup option by ID */
		TidyOption TidyGetOption(HtmlTidyOptionId optId) 
		{
			return tidyGetOption(tdoc, (TidyOptionId)optId);
		};
		/** Lookup option by name */
		TidyOption TidyGetOptionByName(String^ optnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(name, Text::Encoding::UTF8->GetBytes(optnam));
			return tidyGetOptionByName(tdoc, name);
		};

		/** Get ID of given Option */
		HtmlTidyOptionId TidyOptGetId(TidyOption opt) 
		{
			return (HtmlTidyOptionId)tidyOptGetId(opt);
		};

		/** Get name of given Option */
		String^ TidyOptGetName(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetName(opt);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Get datatype of given Option */
		HtmlTidyOptionType TidyOptGetType(TidyOption opt) 
		{
			return (HtmlTidyOptionType)tidyOptGetType(opt);
		};

		/** Is Option read-only? */
		bool TidyOptIsReadOnly(TidyOption opt) 
		{
			return (no != tidyOptIsReadOnly(opt));
		};

		/** Get category of given Option */
		HtmlTidyConfigCategory TidyOptGetCategory(TidyOption opt) 
		{
			return (HtmlTidyConfigCategory)tidyOptGetCategory(opt);
		};

		/** Get default value of given Option as a String^ */
		String^ TidyOptGetDefault(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetDefault(opt);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Get default value of given Option as an unsigned integer */
		ulong TidyOptGetDefaultInt(TidyOption opt) 
		{
			return tidyOptGetDefaultInt(opt);
		};

		/** Get default value of given Option as a boolean value */
		bool TidyOptGetDefaultBool(TidyOption opt) 
		{
			return (no != tidyOptGetDefaultBool(opt));
		};

		/** Iterate over Option "pick list" */
		TidyIterator TidyOptGetPickList(TidyOption opt) 
		{
			return tidyOptGetPickList(opt);
		};

		/** Get next String^ value of Option "pick list" */
		String^ TidyOptGetNextPick(TidyOption opt, TidyIterator* pos) 
		{
			ctmbstr str = tidyOptGetNextPick(opt, pos);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Get current Option value as a String^ */
		String^ TidyOptGetValue(HtmlTidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetValue(tdoc, (TidyOptionId)optId);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Set Option value as a String^ */
		bool TidyOptSetValue(HtmlTidyOptionId optId, String^ val) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(valstr, Text::Encoding::UTF8->GetBytes(val));
			return (no != tidyOptSetValue(tdoc, (TidyOptionId)optId, valstr));
		};

		/** Set named Option value as a string. Good if not sure of type. */
		bool TidyOptParseValue(String^ optnam, String^ val) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(optnamstr, Text::Encoding::UTF8->GetBytes(optnam));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(valstr, Text::Encoding::UTF8->GetBytes(val));
			return (no != tidyOptParseValue(tdoc, optnamstr, valstr));
		};

		/** Get current Option value as an integer */
		ulong TidyOptGetInt(HtmlTidyOptionId optId) 
		{
			return tidyOptGetInt(tdoc, (TidyOptionId)optId);
		};

		/** Set Option value as an integer */
		bool TidyOptSetInt(HtmlTidyOptionId optId, ulong val) 
		{
			return (no != tidyOptSetInt(tdoc, (TidyOptionId)optId, val));
		};

		/** Get current Option value as a boolean flag */
		bool TidyOptGetBool(HtmlTidyOptionId optId) 
		{
			return (no != tidyOptGetBool(tdoc, (TidyOptionId)optId));
		};

		/** Set Option value as a boolean flag */
		bool TidyOptSetBool(HtmlTidyOptionId optId, bool val) 
		{
			return (no != tidyOptSetBool(tdoc, (TidyOptionId)optId, (val ? yes : no)));
		};

		/** Get current Option value as a tristate flag */
		HtmlTidyTriState TidyOptGetAutoBool(HtmlTidyOptionId optId) 
		{
			switch (tidyOptGetInt(tdoc, (TidyOptionId)optId))
			{
			case no:
				return HtmlTidyTriState::NoState;

			case yes:
				return HtmlTidyTriState::YesState;

			default:
				return HtmlTidyTriState::AutoState;
			}
		};

		/** Set Option value as a tristate flag */
		bool TidyOptSetAutoBool(HtmlTidyOptionId optId, HtmlTidyTriState val) 
		{
			int v = TidyAutoState;

			switch (val)
			{
			case HtmlTidyTriState::NoState:
				v = TidyNoState;
				break;

			case HtmlTidyTriState::YesState:
				v = TidyYesState;
				break;

			default:
				v = TidyAutoState;
				break;
			}
			return (no != tidyOptSetInt(tdoc, (TidyOptionId)optId, v));
		};

		/** Reset option to default value by ID */
		bool TidyOptResetToDefault(HtmlTidyOptionId opt) 
		{
			return (no != tidyOptResetToDefault(tdoc, (TidyOptionId)opt));
		};

		/** Reset all options to their default values */
		bool TidyOptResetAllToDefault(void) 
		{
			return (no != tidyOptResetAllToDefault(tdoc));
		};

		/** Take a snapshot of current config settings */
		bool TidyOptSnapshot(void) 
		{
			return (no != tidyOptSnapshot(tdoc));
		};

		/** Reset config settings to snapshot (after document processing) */
		bool TidyOptResetToSnapshot(void) 
		{
			return (no != tidyOptResetToSnapshot(tdoc));
		};

		/** Any settings different than default? */
		bool TidyOptDiffThanDefault(void) 
		{
			return (no != tidyOptDiffThanDefault(tdoc));
		};

		/** Any settings different than snapshot? */
		bool TidyOptDiffThanSnapshot(void) 
		{
			return (no != tidyOptDiffThanSnapshot(tdoc));
		};

		/** Copy current configuration settings from one document to another */
		bool TidyOptCopyConfig(TidyParser^ To) 
		{
			if (To)
			{
				return (no != tidyOptCopyConfig(To->tdoc, tdoc));
			}
			else
			{
				return false;
			}
		};

		/** Get character encoding name. Used with TidyCharEncoding,
		** TidyOutCharEncoding, TidyInCharEncoding */
		String^ TidyOptGetEncName(HtmlTidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetEncName(tdoc, (TidyOptionId)optId);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Get current pick list value for option by ID. Useful for enum types. */
		String^ TidyOptGetCurrPick(HtmlTidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetCurrPick(tdoc, (TidyOptionId)optId);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Iterate over user declared tags */
		TidyIterator TidyOptGetDeclTagList(void) 
		{
			return tidyOptGetDeclTagList(tdoc);
		};

		/** Get next declared tag of specified type: TidyInlineTags, TidyBlockTags,
		** TidyEmptyTags, TidyPreTags */
		String^ TidyOptGetNextDeclTag(HtmlTidyOptionId optId, TidyIterator* iter) 
		{
			ctmbstr str = tidyOptGetNextDeclTag(tdoc, (TidyOptionId)optId, iter);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Get option description */
		String^ TidyOptGetDoc(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetDoc(tdoc, opt);
			String^ ret = gcnew String(str, 0, tmbstrlen(str), Text::Encoding::UTF8);
			return ret;
		};

		/** Iterate over a list of related options */
		TidyIterator TidyOptGetDocLinksList(TidyOption opt) 
		{
			return tidyOptGetDocLinksList(tdoc, opt);
		};

		/** Get next related option */
		TidyOption TidyOptGetNextDocLinks(TidyIterator* pos) 
		{
			return tidyOptGetNextDocLinks(tdoc, pos);
		};

		/** @} end Configuration group */








		/** @defgroup Parse Document Parse
		**
		** Parse markup from a given input source. String and filename 
		** functions added for convenience. HTML/XHTML version determined
		** from input.
		** @{
		*/

		/** Parse markup in named file */
		int TidyParseFile(String^ filename) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Text::Encoding::UTF8->GetBytes(filename));
			return tidyParseFile(tdoc, fname);
		};

		/** Parse markup from the standard input */
		int TidyParseStdin(void) 
		{
			return tidyParseStdin(tdoc);
		};

		/** Parse markup in given String^ */
		int TidyParseString(String^ content) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Text::Encoding::UTF8->GetBytes(content));
			return tidyParseString(tdoc, str);
		};

		/** @} End Parse group */


		/** @defgroup Clean Diagnostics and Repair
		**
		** @{
		*/
		/** Execute configured cleanup and repair operations on parsed markup */
		int TidyCleanAndRepair(void) 
		{
			return tidyCleanAndRepair(tdoc);
		};

		/** Run configured diagnostics on parsed and repaired markup. 
		** Must call TidyCleanAndRepair() first.
		*/
		int TidyRunDiagnostics(void) 
		{
			return tidyRunDiagnostics(tdoc);
		};

		/** @} end Clean group */


		/** @defgroup Save Document Save Functions
		**
		** Save currently parsed document to the given output sink. File name
		** and string/buffer functions provided for convenience.
		** @{
		*/

		/** Save to named file */
		int TidySaveFile(String^ filename) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Text::Encoding::UTF8->GetBytes(filename));
			return tidySaveFile(tdoc, fname);
		};

		/** Save to standard output (FILE*) */
		int TidySaveStdout(void) 
		{
			return tidySaveStdout(tdoc);
		};

		/** Save document to application buffer. If buffer is not big enough,
		** ENOMEM will be returned and the necessary buffer size will be placed
		** in *buflen.
		*/
		int TidySaveString([Out] String^% buffer) // out buffer
		{
			TidyBuffer outbuf = {0};
			TidyOutputSink outsink = {0};
			tidyInitOutputBuffer(&outsink, &outbuf);
			int status = tidySaveSink(tdoc, &outsink);
			if (status >= 0)
			{
				String^ result = gcnew String((ctmbstr)outbuf.bp, 0, outbuf.size, Text::Encoding::UTF8);
				buffer = result;
			}
			tidyBufFree(&outbuf);
			return status;
		};

		/** @} end Save group */


		/** @addtogroup Basic
		** @{
		*/
		/** Save current settings to named file.
		Only non-default values are written. */
		int TidyOptSave([Out] String^% dst) 
		{
			TidyBuffer outbuf = {0};
			int status = tidyOptSaveBuffer(tdoc, &outbuf);
			if (status >= 0)
			{
				String^ result = gcnew String((ctmbstr)outbuf.bp, 0, outbuf.size, Text::Encoding::UTF8);
				dst = result;
			}
			tidyBufFree(&outbuf);
			return status;
		};

		int TidyOptSaveFile(String^ cfgfil) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Text::Encoding::UTF8->GetBytes(cfgfil));
			return tidyOptSaveFile(tdoc, fname);
		};

		bool TidyOptAdjustConfig(void)
		{
			Bool status = tidyOptAdjustConfig(tdoc);

			return (no != status);
		}

		/* Error reporting functions 
		*/

		/** Write more complete information about errors to current error sink. */
		void TidyErrorSummary(void) 
		{
			return tidyErrorSummary(tdoc);
		};

		/** Write more general information about markup to current error sink. */
		void TidyGeneralInfo(void) 
		{
			return tidyGeneralInfo(tdoc);
		};

		/** @} end Basic group (again) */
	};
}
