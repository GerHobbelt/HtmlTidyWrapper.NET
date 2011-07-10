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

#using <mscorlib.dll>
#using <System.Xml.dll>
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

#define TEST_GC_ACTIVITY 1


#if TEST_GC_ACTIVITY

#define VARCONCAT(var1, var2)       VARCONCAT2(var1, var2)
#define VARCONCAT2(var1, var2)      var1 ## var2

#define GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(unmanaged_lvalue, managed_rvalue)									\
			unsigned char VARCONCAT(gbc, __LINE__) __gc[] = managed_rvalue;                                              \
			unsigned char __pin * VARCONCAT(ngbc, __LINE__) = &VARCONCAT(gbc, __LINE__)[0];                                         \
			GC::Collect();                                                                                    \
			GC::WaitForPendingFinalizers();                                                                   \
			ctmbstr unmanaged_lvalue = (ctmbstr)VARCONCAT(ngbc, __LINE__); /* reinterpret_cast<ctmbstr>(ngbc) ? */       

#else

#define GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(unmanaged_lvalue, managed_rvalue)									\
			unsigned char VARCONCAT(gbc, __LINE__) __gc[] = managed_rvalue;                                              \
			unsigned char __pin * VARCONCAT(ngbc, __LINE__) = &VARCONCAT(gbc, __LINE__)[0];                                         \
			ctmbstr unmanaged_lvalue = (ctmbstr)VARCONCAT(ngbc, __LINE__); /* reinterpret_cast<ctmbstr>(ngbc) ? */       

#endif






namespace HtmlTidy 
{
#include "tidy.h"
#include "buffio.h"

#include "HtmlTidyWrapper.h"




/** Categories of Tidy configuration options
*/
public enum class HtmlTidyConfigCategory
{
  Markup,          /**< Markup options: (X)HTML version, etc */
  Diagnostics,     /**< Diagnostics */
  PrettyPrint,     /**< Output layout */
  Encoding,        /**< Character encodings */
  Miscellaneous    /**< File handling, message format, etc. */
};


/** Option IDs Used to get/set option values.
*/
public enum class HtmlTidyOptionId
{
  UnknownOption,   /**< Unknown option! */
  IndentSpaces,    /**< Indentation n spaces */
  WrapLen,         /**< Wrap margin */
  TabSize,         /**< Expand tabs to n spaces */

  CharEncoding,    /**< In/out character encoding */
  InCharEncoding,  /**< Input character encoding (if different) */
  OutCharEncoding, /**< Output character encoding (if different) */    
  Newline,         /**< Output line ending (default to platform) */

  DoctypeMode,     /**< See doctype property */
  Doctype,         /**< User specified doctype */

  DuplicateAttrs,  /**< Keep first or last duplicate attribute */
  AltText,         /**< Default text for alt attribute */
  
  /* obsolete */
  SlideStyle,      /**< Style sheet for slides: not used for anything yet */

  ErrFile,         /**< File name to write errors to */
  OutFile,         /**< File name to write markup to */
  WriteBack,       /**< If true then output tidied markup */
  ShowMarkup,      /**< If false, normal output is suppressed */
  ShowWarnings,    /**< However errors are always shown */
  Quiet,           /**< No 'Parsing X', guessed DTD or summary */
  IndentContent,   /**< Indent content of appropriate tags */
                       /**< "auto" does text/block level content indentation */
  HideEndTags,     /**< Suppress optional end tags */
  XmlTags,         /**< Treat input as XML */
  XmlOut,          /**< Create output as XML */
  XhtmlOut,        /**< Output extensible HTML */
  HtmlOut,         /**< Output plain HTML, even for XHTML input.
                           Yes means set explicitly. */
  XmlDecl,         /**< Add <?xml?> for XML docs */
  UpperCaseTags,   /**< Output tags in upper not lower case */
  UpperCaseAttrs,  /**< Output attributes in upper not lower case */
  MakeBare,        /**< Make bare HTML: remove Microsoft cruft */
  MakeClean,       /**< Replace presentational clutter by style rules */
  LogicalEmphasis, /**< Replace i by em and b by strong */
  DropPropAttrs,   /**< Discard proprietary attributes */
  DropFontTags,    /**< Discard presentation tags */
  DropEmptyParas,  /**< Discard empty p elements */
  DropEmptyOptions,/**< Discard empty option elements [i_a] */
  FixComments,     /**< Fix comments with adjacent hyphens */
  BreakBeforeBR,   /**< Output newline before <br> or not? */

  /* obsolete */
  BurstSlides,     /**< Create slides on each h2 element */

  NumEntities,     /**< Use numeric entities */
  QuoteMarks,      /**< Output " marks as &quot; */
  QuoteNbsp,       /**< Output non-breaking space as entity */
  QuoteAmpersand,  /**< Output naked ampersand as &amp; */
  WrapAttVals,     /**< Wrap within attribute values */
  WrapScriptlets,  /**< Wrap within JavaScript string literals */
  WrapSection,     /**< Wrap within <![ ... ]> section tags */
  WrapAsp,         /**< Wrap within ASP pseudo elements */
  WrapJste,        /**< Wrap within JSTE pseudo elements */
  WrapPhp,         /**< Wrap within PHP pseudo elements */
  FixBackslash,    /**< Fix URLs by replacing \ with / */
  IndentAttributes,/**< Newline+indent before each attribute */
  XmlPIs,          /**< If set to yes PIs must end with ?> */
  XmlSpace,        /**< If set to yes adds xml:space attr as needed */
  EncloseBodyText, /**< If yes text at body is wrapped in P's */
  EncloseBlockText,/**< If yes text in blocks is wrapped in P's */
  KeepFileTimes,   /**< If yes last modied time is preserved */
  Word2000,        /**< Draconian cleaning for Word2000 */
  Mark,            /**< Add meta element indicating tidied doc */
  Emacs,           /**< If true format error output for GNU Emacs */
  EmacsFile,       /**< Name of current Emacs file */
  LiteralAttribs,  /**< If true attributes may use newlines */
  BodyOnly,        /**< Output BODY content only */
  FixUri,          /**< Applies URI encoding if necessary */
  LowerLiterals,   /**< Folds known attribute values to lower case */
  HideComments,    /**< Hides all (real) comments in output */
  IndentCdata,     /**< Indent <!CDATA[ ... ]]> section */
  ForceOutput,     /**< Output document even if errors were found */
  ShowErrors,      /**< Number of errors to put out */
  AsciiChars,      /**< Convert quotes and dashes to nearest ASCII char */
  JoinClasses,     /**< Join multiple class attributes */
  JoinStyles,      /**< Join multiple style attributes */
  EscapeCdata,     /**< Replace <![CDATA[]]> sections with escaped text */

#if SUPPORT_ASIAN_ENCODINGS
  Language,        /**< Language property: not used for anything yet */
  NCR,             /**< Allow numeric character references */
#else
  LanguageNotUsed,
  NCRNotUsed,
#endif
#if SUPPORT_UTF16_ENCODINGS
  OutputBOM,      /**< Output a Byte Order Mark (BOM) for UTF-16 encodings */
                      /**< auto: if input stream has BOM, we output a BOM */
#else
  OutputBOMNotUsed,
#endif

  ReplaceColor,    /**< Replace hex color attribute values with names */
  CSSPrefix,       /**< CSS class naming for -clean option */

  InlineTags,      /**< Declared inline tags */
  BlockTags,       /**< Declared block tags */
  EmptyTags,       /**< Declared empty tags */
  PreTags,         /**< Declared pre tags */

  AccessibilityCheckLevel, /**< Accessibility check level 
                                   0 (old style), or 1, 2, 3 */

  VertSpace,       /**< degree to which markup is spread out vertically */
#if SUPPORT_ASIAN_ENCODINGS
  PunctWrap,       /**< consider punctuation and breaking spaces for wrapping */
#else
  PunctWrapNotUsed,
#endif
  MergeDivs,       /**< Merge multiple DIVs */
  DecorateInferredUL,  /**< Mark inferred UL elements with no indent CSS */
  PreserveEntities,    /**< Preserve entities */
  SortAttributes,      /**< Sort attributes */
  MergeSpans,       /**< Merge multiple SPANs */
  AnchorAsName,    /**< Define anchors as name attributes */
  FixTitle,            /**< Fix an empty title element by filling it with a copy of the first header element in the document body */ 
  N_OPTIONS       /**< Must be last */
};

/** Option data types
*/
public enum class HtmlTidyOptionType
{
  String,          /**< String */
  Integer,         /**< Integer or enumeration */
  Boolean          /**< Boolean flag */
};


/** AutoBool values used by ParseBool, ParseTriState, ParseIndent, ParseBOM
*/
public enum class HtmlTidyTriState
{
   NoState,     /**< maps to 'no' */
   YesState,    /**< maps to 'yes' */
   AutoState    /**< Automatic */
};

/** TidyNewline option values to control output line endings.
*/
public enum class HtmlTidyLineEnding
{
    LF,         /**< Use Unix style: LF */
    CRLF,       /**< Use DOS/Windows style: CR+LF */
    CR          /**< Use Macintosh style: CR */
};


/** Mode controlling treatment of doctype
*/
public enum class HtmlTidyDoctypeModes
{
    DoctypeOmit,    /**< Omit DOCTYPE altogether */
    DoctypeAuto,    /**< Keep DOCTYPE in input.  Set version to content */
    DoctypeStrict,  /**< Convert document to HTML 4 strict content model */
    DoctypeLoose,   /**< Convert document to HTML 4 transitional
                             content model */
    DoctypeUser     /**< Set DOCTYPE FPI explicitly */
};

/** Mode controlling treatment of duplicate Attributes
*/
public enum class HtmlTidyDupAttrModes
{
    KeepFirst,
    KeepLast
};

/** Mode controlling treatment of sorting attributes
*/
public enum class HtmlTidyAttrSortStrategy
{
    SortAttrNone,
    SortAttrAlpha
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
  Info,             /**< Information about markup usage */
  Warning,          /**< Warning message */
  Config,           /**< Configuration error */
  Access,           /**< Accessibility message */
  Error,            /**< Error message - output suppressed */
  BadDocument,      /**< I/O or file system error */
  Fatal             /**< Crash! */
};


/* Document tree traversal functions
*/

/** Node types
*/
public enum class HtmlTidyNodeType
{
  Root,        /**< Root */
  DocType,     /**< DOCTYPE */
  Comment,     /**< Comment */
  ProcIns,     /**< Processing Instruction */
  Text,        /**< Text */
  Start,       /**< Start Tag */
  End,         /**< End Tag */
  StartEnd,    /**< Start/End (empty) Tag */
  CDATA,       /**< Unparsed Text */
  Section,     /**< XML Section */
  Asp,         /**< ASP Source */
  Jste,        /**< JSTE Source */
  Php,         /**< PHP Source */
  XmlDecl      /**< XML Declaration */
};


/** Known HTML element types
*/
public enum class HtmlTidyTagId
{
  UNKNOWN,  /**< Unknown tag! */
  A,        /**< A */
  ABBR,     /**< ABBR */
  ACRONYM,  /**< ACRONYM */
  ADDRESS,  /**< ADDRESS */
  ALIGN,    /**< ALIGN */
  APPLET,   /**< APPLET */
  AREA,     /**< AREA */
  B,        /**< B */
  BASE,     /**< BASE */
  BASEFONT, /**< BASEFONT */
  BDO,      /**< BDO */
  BGSOUND,  /**< BGSOUND */
  BIG,      /**< BIG */
  BLINK,    /**< BLINK */
  BLOCKQUOTE,   /**< BLOCKQUOTE */
  BODY,     /**< BODY */
  BR,       /**< BR */
  BUTTON,   /**< BUTTON */
  CAPTION,  /**< CAPTION */
  CENTER,   /**< CENTER */
  CITE,     /**< CITE */
  CODE,     /**< CODE */
  COL,      /**< COL */
  COLGROUP, /**< COLGROUP */
  COMMENT,  /**< COMMENT */
  DD,       /**< DD */
  DEL,      /**< DEL */
  DFN,      /**< DFN */
  DIR,      /**< DIR */
  DIV,      /**< DIF */
  DL,       /**< DL */
  DT,       /**< DT */
  EM,       /**< EM */
  EMBED,    /**< EMBED */
  FIELDSET, /**< FIELDSET */
  FONT,     /**< FONT */
  FORM,     /**< FORM */
  FRAME,    /**< FRAME */
  FRAMESET, /**< FRAMESET */
  H1,       /**< H1 */
  H2,       /**< H2 */
  H3,       /**< H3 */
  H4,       /**< H4 */
  H5,       /**< H5 */
  H6,       /**< H6 */
  HEAD,     /**< HEAD */
  HR,       /**< HR */
  HTML,     /**< HTML */
  I,        /**< I */
  IFRAME,   /**< IFRAME */
  ILAYER,   /**< ILAYER */
  IMG,      /**< IMG */
  INPUT,    /**< INPUT */
  INS,      /**< INS */
  ISINDEX,  /**< ISINDEX */
  KBD,      /**< KBD */
  KEYGEN,   /**< KEYGEN */
  LABEL,    /**< LABEL */
  LAYER,    /**< LAYER */
  LEGEND,   /**< LEGEND */
  LI,       /**< LI */
  LINK,     /**< LINK */
  LISTING,  /**< LISTING */
  MAP,      /**< MAP */
  MARQUEE,  /**< MARQUEE */
  MENU,     /**< MENU */
  META,     /**< META */
  MULTICOL, /**< MULTICOL */
  NOBR,     /**< NOBR */
  NOEMBED,  /**< NOEMBED */
  NOFRAMES, /**< NOFRAMES */
  NOLAYER,  /**< NOLAYER */
  NOSAVE,   /**< NOSAVE */
  NOSCRIPT, /**< NOSCRIPT */
  OBJECT,   /**< OBJECT */
  OL,       /**< OL */
  OPTGROUP, /**< OPTGROUP */
  OPTION,   /**< OPTION */
  P,        /**< P */
  PARAM,    /**< PARAM */
  PLAINTEXT,/**< PLAINTEXT */
  PRE,      /**< PRE */
  Q,        /**< Q */
  RB,       /**< RB */
  RBC,      /**< RBC */
  RP,       /**< RP */
  RT,       /**< RT */
  RTC,      /**< RTC */
  RUBY,     /**< RUBY */
  S,        /**< S */
  SAMP,     /**< SAMP */
  SCRIPT,   /**< SCRIPT */
  SELECT,   /**< SELECT */
  SERVER,   /**< SERVER */
  SERVLET,  /**< SERVLET */
  SMALL,    /**< SMALL */
  SPACER,   /**< SPACER */
  SPAN,     /**< SPAN */
  STRIKE,   /**< STRIKE */
  STRONG,   /**< STRONG */
  STYLE,    /**< STYLE */
  SUB,      /**< SUB */
  SUP,      /**< SUP */
  TABLE,    /**< TABLE */
  TBODY,    /**< TBODY */
  TD,       /**< TD */
  TEXTAREA, /**< TEXTAREA */
  TFOOT,    /**< TFOOT */
  TH,       /**< TH */
  THEAD,    /**< THEAD */
  TITLE,    /**< TITLE */
  TR,       /**< TR */
  TT,       /**< TT */
  U,        /**< U */
  UL,       /**< UL */
  VAR,      /**< VAR */
  WBR,      /**< WBR */
  XMP,      /**< XMP */
  NEXTID,   /**< NEXTID */

  N_TAGS       /**< Must be last */
};

/* Attribute interrogation
*/

/** Known HTML attributes
*/
public enum class HtmlTidyAttrId
{
  UNKNOWN,           /**< UNKNOWN= */
  ABBR,              /**< ABBR= */
  ACCEPT,            /**< ACCEPT= */
  ACCEPT_CHARSET,    /**< ACCEPT_CHARSET= */
  ACCESSKEY,         /**< ACCESSKEY= */
  ACTION,            /**< ACTION= */
  ADD_DATE,          /**< ADD_DATE= */
  ALIGN,             /**< ALIGN= */
  ALINK,             /**< ALINK= */
  ALT,               /**< ALT= */
  ARCHIVE,           /**< ARCHIVE= */
  AXIS,              /**< AXIS= */
  BACKGROUND,        /**< BACKGROUND= */
  BGCOLOR,           /**< BGCOLOR= */
  BGPROPERTIES,      /**< BGPROPERTIES= */
  BORDER,            /**< BORDER= */
  BORDERCOLOR,       /**< BORDERCOLOR= */
  BOTTOMMARGIN,      /**< BOTTOMMARGIN= */
  CELLPADDING,       /**< CELLPADDING= */
  CELLSPACING,       /**< CELLSPACING= */
  CHAR,              /**< CHAR= */
  CHAROFF,           /**< CHAROFF= */
  CHARSET,           /**< CHARSET= */
  CHECKED,           /**< CHECKED= */
  CITE,              /**< CITE= */
  CLASS,             /**< CLASS= */
  CLASSID,           /**< CLASSID= */
  CLEAR,             /**< CLEAR= */
  CODE,              /**< CODE= */
  CODEBASE,          /**< CODEBASE= */
  CODETYPE,          /**< CODETYPE= */
  COLOR,             /**< COLOR= */
  COLS,              /**< COLS= */
  COLSPAN,           /**< COLSPAN= */
  COMPACT,           /**< COMPACT= */
  CONTENT,           /**< CONTENT= */
  COORDS,            /**< COORDS= */
  DATA,              /**< DATA= */
  DATAFLD,           /**< DATAFLD= */
  DATAFORMATAS,      /**< DATAFORMATAS= */
  DATAPAGESIZE,      /**< DATAPAGESIZE= */
  DATASRC,           /**< DATASRC= */
  DATETIME,          /**< DATETIME= */
  DECLARE,           /**< DECLARE= */
  DEFER,             /**< DEFER= */
  DIR,               /**< DIR= */
  DISABLED,          /**< DISABLED= */
  ENCODING,          /**< ENCODING= */
  ENCTYPE,           /**< ENCTYPE= */
  FACE,              /**< FACE= */
  FOR,               /**< FOR= */
  FRAME,             /**< FRAME= */
  FRAMEBORDER,       /**< FRAMEBORDER= */
  FRAMESPACING,      /**< FRAMESPACING= */
  GRIDX,             /**< GRIDX= */
  GRIDY,             /**< GRIDY= */
  HEADERS,           /**< HEADERS= */
  HEIGHT,            /**< HEIGHT= */
  HREF,              /**< HREF= */
  HREFLANG,          /**< HREFLANG= */
  HSPACE,            /**< HSPACE= */
  HTTP_EQUIV,        /**< HTTP_EQUIV= */
  ID,                /**< ID= */
  ISMAP,             /**< ISMAP= */
  LABEL,             /**< LABEL= */
  LANG,              /**< LANG= */
  LANGUAGE,          /**< LANGUAGE= */
  LAST_MODIFIED,     /**< LAST_MODIFIED= */
  LAST_VISIT,        /**< LAST_VISIT= */
  LEFTMARGIN,        /**< LEFTMARGIN= */
  LINK,              /**< LINK= */
  LONGDESC,          /**< LONGDESC= */
  LOWSRC,            /**< LOWSRC= */
  MARGINHEIGHT,      /**< MARGINHEIGHT= */
  MARGINWIDTH,       /**< MARGINWIDTH= */
  MAXLENGTH,         /**< MAXLENGTH= */
  MEDIA,             /**< MEDIA= */
  METHOD,            /**< METHOD= */
  MULTIPLE,          /**< MULTIPLE= */
  NAME,              /**< NAME= */
  NOHREF,            /**< NOHREF= */
  NORESIZE,          /**< NORESIZE= */
  NOSHADE,           /**< NOSHADE= */
  NOWRAP,            /**< NOWRAP= */
  OBJECT,            /**< OBJECT= */
  OnAFTERUPDATE,     /**< OnAFTERUPDATE= */
  OnBEFOREUNLOAD,    /**< OnBEFOREUNLOAD= */
  OnBEFOREUPDATE,    /**< OnBEFOREUPDATE= */
  OnBLUR,            /**< OnBLUR= */
  OnCHANGE,          /**< OnCHANGE= */
  OnCLICK,           /**< OnCLICK= */
  OnDATAAVAILABLE,   /**< OnDATAAVAILABLE= */
  OnDATASETCHANGED,  /**< OnDATASETCHANGED= */
  OnDATASETCOMPLETE, /**< OnDATASETCOMPLETE= */
  OnDBLCLICK,        /**< OnDBLCLICK= */
  OnERRORUPDATE,     /**< OnERRORUPDATE= */
  OnFOCUS,           /**< OnFOCUS= */
  OnKEYDOWN,         /**< OnKEYDOWN= */
  OnKEYPRESS,        /**< OnKEYPRESS= */
  OnKEYUP,           /**< OnKEYUP= */
  OnLOAD,            /**< OnLOAD= */
  OnMOUSEDOWN,       /**< OnMOUSEDOWN= */
  OnMOUSEMOVE,       /**< OnMOUSEMOVE= */
  OnMOUSEOUT,        /**< OnMOUSEOUT= */
  OnMOUSEOVER,       /**< OnMOUSEOVER= */
  OnMOUSEUP,         /**< OnMOUSEUP= */
  OnRESET,           /**< OnRESET= */
  OnROWENTER,        /**< OnROWENTER= */
  OnROWEXIT,         /**< OnROWEXIT= */
  OnSELECT,          /**< OnSELECT= */
  OnSUBMIT,          /**< OnSUBMIT= */
  OnUNLOAD,          /**< OnUNLOAD= */
  PROFILE,           /**< PROFILE= */
  PROMPT,            /**< PROMPT= */
  RBSPAN,            /**< RBSPAN= */
  READONLY,          /**< READONLY= */
  REL,               /**< REL= */
  REV,               /**< REV= */
  RIGHTMARGIN,       /**< RIGHTMARGIN= */
  ROWS,              /**< ROWS= */
  ROWSPAN,           /**< ROWSPAN= */
  RULES,             /**< RULES= */
  SCHEME,            /**< SCHEME= */
  SCOPE,             /**< SCOPE= */
  SCROLLING,         /**< SCROLLING= */
  SELECTED,          /**< SELECTED= */
  SHAPE,             /**< SHAPE= */
  SHOWGRID,          /**< SHOWGRID= */
  SHOWGRIDX,         /**< SHOWGRIDX= */
  SHOWGRIDY,         /**< SHOWGRIDY= */
  SIZE,              /**< SIZE= */
  SPAN,              /**< SPAN= */
  SRC,               /**< SRC= */
  STANDBY,           /**< STANDBY= */
  START,             /**< START= */
  STYLE,             /**< STYLE= */
  SUMMARY,           /**< SUMMARY= */
  TABINDEX,          /**< TABINDEX= */
  TARGET,            /**< TARGET= */
  TEXT,              /**< TEXT= */
  TITLE,             /**< TITLE= */
  TOPMARGIN,         /**< TOPMARGIN= */
  TYPE,              /**< TYPE= */
  USEMAP,            /**< USEMAP= */
  VALIGN,            /**< VALIGN= */
  VALUE,             /**< VALUE= */
  VALUETYPE,         /**< VALUETYPE= */
  VERSION,           /**< VERSION= */
  VLINK,             /**< VLINK= */
  VSPACE,            /**< VSPACE= */
  WIDTH,             /**< WIDTH= */
  WRAP,              /**< WRAP= */
  XML_LANG,          /**< XML_LANG= */
  XML_SPACE,         /**< XML_SPACE= */
  XMLNS,             /**< XMLNS= */

  EVENT,             /**< EVENT= */
  METHODS,           /**< METHODS= */
  N,                 /**< N= */
  SDAFORM,           /**< SDAFORM= */
  SDAPREF,           /**< SDAPREF= */
  SDASUFF,           /**< SDASUFF= */
  URN,               /**< URN= */

  N_ATTRIBS              /**< Must be last */
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
  Normal = 0x0000,	/**< NORMAL (default) */ 
  Preformatted = 0x0001,	/**< PREFORMATTED */
  Comment = 0x0002,	/**< COMMENT */
  AttribValue = 0x0004,	/**< ATTRIBVALUE */
  NoWrap = 0x0008,	/**< NOWRAP */
  CDATA = 0x0010,	/**< CDATA */
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
	private __gc class DummyUrlResolver: public XmlUrlResolver 
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




	public __gc class TidyParser
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
					attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
				}
				if (tattval != NULL)
				{
					attval = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
				}

				/*
				* Attribute Attribute Interrogation
				*
				* Get information about any given attribute.
				*/

				TidyAttrId id = tidyAttrGetId(att);
				Bool AttrIsEvent = tidyAttrIsEvent(att);
				Bool AttrIsProp = tidyAttrIsProp(att);

				XmlAttribute^ a = doc->CreateAttribute(attname);
				if (attval != nullptr)
				{
					a->Value = attval;
				}
				e->Attributes->Append(a);
			}
		}




		void process_tree(TidyNode node, XmlDocument __gc * doc, XmlNode __gc * current)
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

				ctmbstr nodename = tidyNodeGetName(n);
				Bool istext = tidyNodeIsText(n);
				Bool isprop = tidyNodeIsProp(tdoc, n);
				Bool isheader = tidyNodeIsHeader(n);
				Bool hastext = tidyNodeHasText(tdoc, n);
				TidyBuffer textbuf = {0};
				Bool gettxt = tidyNodeGetText(tdoc, n, &textbuf);
				String __gc * textstr = String::Empty;
				if (gettxt && textbuf.bp)
				{
					textstr = __gc new String((ctmbstr )textbuf.bp, 0, textbuf.size, Encoding::UTF8);
				}
				TidyTagId tagid = tidyNodeGetId(n);
				uint line = tidyNodeLine(n);
				uint column = tidyNodeColumn(n);
				Bool ishtml = tidyNodeIsHTML(n);
				Bool ishead = tidyNodeIsHEAD(n);
				Bool IsTITLE = tidyNodeIsTITLE(n);
				Bool IsBASE = tidyNodeIsBASE(n);
				Bool IsMETA = tidyNodeIsMETA(n);
				Bool IsBODY = tidyNodeIsBODY(n);
				Bool IsFRAMESET = tidyNodeIsFRAMESET(n);
				Bool IsFRAME = tidyNodeIsFRAME(n);
				Bool IsIFRAME = tidyNodeIsIFRAME(n);
				Bool IsNOFRAMES = tidyNodeIsNOFRAMES(n);
				Bool IsHR = tidyNodeIsHR(n);
				Bool IsH1 = tidyNodeIsH1(n);
				Bool IsH2 = tidyNodeIsH2(n);
				Bool IsPRE = tidyNodeIsPRE(n);
				Bool IsLISTING = tidyNodeIsLISTING(n);
				Bool IsP = tidyNodeIsP(n);
				Bool IsUL = tidyNodeIsUL(n);
				Bool IsOL = tidyNodeIsOL(n);
				Bool IsDL = tidyNodeIsDL(n);
				Bool IsDIR = tidyNodeIsDIR(n);
				Bool IsLI = tidyNodeIsLI(n);
				Bool IsDT = tidyNodeIsDT(n);
				Bool IsDD = tidyNodeIsDD(n);
				Bool IsTABLE = tidyNodeIsTABLE(n);
				Bool IsCAPTION = tidyNodeIsCAPTION(n);
				Bool IsTD = tidyNodeIsTD(n);
				Bool IsTH = tidyNodeIsTH(n);
				Bool IsTR = tidyNodeIsTR(n);
				Bool IsCOL = tidyNodeIsCOL(n);
				Bool IsCOLGROUP = tidyNodeIsCOLGROUP(n);
				Bool IsBR = tidyNodeIsBR(n);
				Bool IsA = tidyNodeIsA(n);
				Bool IsLINK = tidyNodeIsLINK(n);
				Bool IsB = tidyNodeIsB(n);
				Bool IsI = tidyNodeIsI(n);
				Bool IsSTRONG = tidyNodeIsSTRONG(n);
				Bool IsEM = tidyNodeIsEM(n);
				Bool IsBIG = tidyNodeIsBIG(n);
				Bool IsSMALL = tidyNodeIsSMALL(n);
				Bool IsPARAM = tidyNodeIsPARAM(n);
				Bool IsOPTION = tidyNodeIsOPTION(n);
				Bool IsOPTGROUP = tidyNodeIsOPTGROUP(n);
				Bool IsIMG = tidyNodeIsIMG(n);
				Bool IsMAP = tidyNodeIsMAP(n);
				Bool IsAREA = tidyNodeIsAREA(n);
				Bool IsNOBR = tidyNodeIsNOBR(n);
				Bool IsWBR = tidyNodeIsWBR(n);
				Bool IsFONT = tidyNodeIsFONT(n);
				Bool IsLAYER = tidyNodeIsLAYER(n);
				Bool IsSPACER = tidyNodeIsSPACER(n);
				Bool IsCENTER = tidyNodeIsCENTER(n);
				Bool IsSTYLE = tidyNodeIsSTYLE(n);
				Bool IsSCRIPT = tidyNodeIsSCRIPT(n);
				Bool IsNOSCRIPT = tidyNodeIsNOSCRIPT(n);
				Bool IsFORM = tidyNodeIsFORM(n);
				Bool IsTEXTAREA = tidyNodeIsTEXTAREA(n);
				Bool IsBLOCKQUOTE = tidyNodeIsBLOCKQUOTE(n);
				Bool IsAPPLET = tidyNodeIsAPPLET(n);
				Bool IsOBJECT = tidyNodeIsOBJECT(n);
				Bool IsDIV = tidyNodeIsDIV(n);
				Bool IsSPAN = tidyNodeIsSPAN(n);
				Bool IsINPUT = tidyNodeIsINPUT(n);
				Bool IsQ = tidyNodeIsQ(n);
				Bool IsLABEL = tidyNodeIsLABEL(n);
				Bool IsH3 = tidyNodeIsH3(n);
				Bool IsH4 = tidyNodeIsH4(n);
				Bool IsH5 = tidyNodeIsH5(n);
				Bool IsH6 = tidyNodeIsH6(n);
				Bool IsADDRESS = tidyNodeIsADDRESS(n);
				Bool IsXMP = tidyNodeIsXMP(n);
				Bool IsSELECT = tidyNodeIsSELECT(n);
				Bool IsBLINK = tidyNodeIsBLINK(n);
				Bool IsMARQUEE = tidyNodeIsMARQUEE(n);
				Bool IsEMBED = tidyNodeIsEMBED(n);
				Bool IsBASEFONT = tidyNodeIsBASEFONT(n);
				Bool IsISINDEX = tidyNodeIsISINDEX(n);
				Bool IsS = tidyNodeIsS(n);
				Bool IsSTRIKE = tidyNodeIsSTRIKE(n);
				Bool IsU = tidyNodeIsU(n);
				Bool IsMENU = tidyNodeIsMENU(n);


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
							name = gcnew String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

							XmlNode^ e = doc->CreateElement(name);

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
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);
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
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
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
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
							}
							sysatt = attval;
						}

						XmlDocumentType __gc * e = nullptr;
						try
						{
							e = doc->CreateDocumentType(name, pubatt, sysatt, nullptr);
						}
						catch (Exception __gc * ex)
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
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlComment __gc * e = doc->CreateComment(textstr);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_ProcIns:     /* Processing Instruction */
					{
						ctmbstr tname = "Processing Instruction";
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateProcessingInstruction(name, String::Empty);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Text:        /* Text */
					{
						ctmbstr tname = "Text";                    
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateTextNode(textstr);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_CDATA:       /* Unparsed Text */
					{
						ctmbstr tname = "CDATA";                   
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateCDataSection(textstr);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Section:     /* XML Section */
					{
						ctmbstr tname = "XML Section";             
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Asp:         /* ASP Source */
					{
						ctmbstr tname = "ASP";                     
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Jste:        /* JSTE Source */
					{
						ctmbstr tname = "JSTE";                    
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_Php:         /* PHP Source */
					{
						ctmbstr tname = "PHP";                     
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

						XmlNode __gc * e = doc->CreateElement(name);

						CopyAllAttributes(e, doc, n);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}
				case TidyNode_XmlDecl:     /* XML Declaration */
					{
						ctmbstr tname = "XML Declaration";         
						String __gc * name = __gc new String(tname, 0, tmbstrlen(tname), Encoding::UTF8);
						TidyAttr ver = tidyAttrGetById(n, TidyAttr_VERSION);
						TidyAttr enc = tidyAttrGetById(n, TidyAttr_ENCODING);
						TidyAttr sa = tidyGetAttrByName(n, "standalone");
						String __gc * veratt = String::Empty;
						String __gc * encatt = String::Empty;
						String __gc * saatt = String::Empty;
						if (ver)
						{
							ctmbstr tattname = tidyAttrName(ver);
							ctmbstr tattval = tidyAttrValue(ver);
							String^ attname = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
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
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattname)
							{
								attname = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
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
								attname = gcnew String(tattname, 0, tmbstrlen(tattname), Encoding::UTF8);
							}
							String^ attval = nullptr;
							if (tattval)
							{
								attval = gcnew String(tattval, 0, tmbstrlen(tattval), Encoding::UTF8);
							}
							saatt = attval;
						}

						XmlNode __gc * e = doc->CreateXmlDeclaration(veratt, encatt, saatt);

						current->AppendChild(e);
						process_tree(child, doc, e);

						break;
					}

				case TidyNode_End:         /* End Tag */
					{
						ctmbstr tname = tidyNodeGetName(n);
						if (tname)
						{
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

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
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

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
							String^ name = gcnew String(tname, 0, tmbstrlen(tname), Encoding::UTF8);

							XmlNode^ e = doc->CreateElement(name);

							CopyAllAttributes(e, doc, n);

							current->AppendChild(e);
							process_tree(child, doc, e);
						}
						else
						{
							throw gcnew Exception(String::Format("Tidy Node type {0} has no name", __box(t)));
						}
						break;
					}
				}

				n = tidyGetNext(n);
			}
		}




	public:
		TidyParser()
		{
			errbuf = tidyBufCreate();
			if (errbuf)
			{
				tidyBufAlloc(errbuf, 2048);
			}
			errsink = tidyOutputSinkCreate();
			tidyInitOutputBuffer(errsink, errbuf);

			tdoc = tidyCreate();

			rc = tidySetErrorSink(tdoc, errsink);   // Capture diagnostics
			if (rc < 0)
			{
				tidyRelease(tdoc);
				tdoc = 0;
			}
		};

		~TidyParser()
		{
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
			if (errsink != 0)
			{
				tidyOutputSinkDestroy(errsink);
				errsink = 0;
			}
		};

		void Close()
		{
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
			if (errsink != 0)
			{
				tidyOutputSinkDestroy(errsink);
				errsink = 0;
			}
		};


		String __gc * ErrorMessage(void)
		{
			if (errbuf)
			{
				String __gc * ret = __gc new String((ctmbstr )errbuf->bp, 0, errbuf->size, Encoding::UTF8);

				return ret;
			}
			else
			{
				String __gc * ret = String::Empty;

				return ret;
			}
		};



		XmlDocument __gc * DoParseHtml(String __gc * s)
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(input, Encoding::UTF8->GetBytes(s));

			tidyParseString(tdoc, input);
			tidyCleanAndRepair(tdoc);
			tidyRunDiagnostics(tdoc);

			TidyNode root = tidyGetRoot(tdoc);

			XmlDocument __gc * doc = __gc new XmlDocument();
			doc->XmlResolver = __gc new DummyUrlResolver();
			XmlNode __gc * node;
			process_tree(root, doc, node);

			return doc;
		}

		/** Get release date (version) for current library */
		String __gc * TidyReleaseDate(void)
		{
			ctmbstr str = tidyReleaseDate();
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);

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
		int TidyLoadConfigFromString(String __gc * config) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Encoding::UTF8->GetBytes(config));
			TidyBuffer buffer = {0};
			tidyBufAttach(&buffer, (byte *)str, tmbstrlen(str) + 1);

			int ret = tidyLoadConfigBuffer(tdoc, &buffer);
			tidyBufDetach(&buffer);

			return ret;
		};

		int TidyLoadConfigFromFile(String __gc * configfile) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Encoding::UTF8->GetBytes(configfile));
			return tidyLoadConfig(tdoc, fname);
		};

		/** Load a Tidy configuration, stored in the config string, with the specified character encoding */
		int TidyLoadConfigFromStringEnc(String __gc * config, String __gc * charenc) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Encoding::UTF8->GetBytes(config));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Encoding::UTF8->GetBytes(config));
			TidyBuffer buffer = {0};
			tidyBufAttach(&buffer, (byte *)str, tmbstrlen(str) + 1);

			int ret = tidyLoadConfigBufferEnc(tdoc, &buffer, enc);
			tidyBufDetach(&buffer);

			return ret;
		};

		int TidyLoadConfigEncFromFile(String __gc * configfile, String __gc * charenc) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Encoding::UTF8->GetBytes(configfile));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Encoding::UTF8->GetBytes(charenc));
			return tidyLoadConfigEnc(tdoc, fname, enc);
		};



		/** Set the input/output character encoding for parsing markup.
		** Values include: ascii, latin1, raw, utf8, iso2022, mac,
		** win1252, utf16le, utf16be, utf16, big5 and shiftjis. Case in-sensitive.
		*/
		int TidySetCharEncoding(String __gc * encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Encoding::UTF8->GetBytes(encnam));
			return tidySetCharEncoding(tdoc, enc);
		};

		/** Set the input encoding for parsing markup.
		** As for TidySetCharEncoding but only affects the input encoding
		**/
		int TidySetInCharEncoding(String __gc * encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Encoding::UTF8->GetBytes(encnam));
			return tidySetInCharEncoding(tdoc, enc);
		};

		/** Set the output encoding.
		**/
		int TidySetOutCharEncoding(String __gc * encnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(enc, Encoding::UTF8->GetBytes(encnam));
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
		TidyOptionId TidyOptGetIdForName(String __gc * optnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(name, Encoding::UTF8->GetBytes(optnam));
			return tidyOptGetIdForName(name);
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
		TidyOption TidyGetOption(TidyOptionId optId) 
		{
			return tidyGetOption(tdoc, optId);
		};
		/** Lookup option by name */
		TidyOption TidyGetOptionByName(String __gc * optnam) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(name, Encoding::UTF8->GetBytes(optnam));
			return tidyGetOptionByName(tdoc, name);
		};

		/** Get ID of given Option */
		TidyOptionId TidyOptGetId(TidyOption opt) 
		{
			return tidyOptGetId(opt);
		};

		/** Get name of given Option */
		String __gc * TidyOptGetName(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetName(opt);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Get datatype of given Option */
		TidyOptionType TidyOptGetType(TidyOption opt) 
		{
			return tidyOptGetType(opt);
		};

		/** Is Option read-only? */
		bool TidyOptIsReadOnly(TidyOption opt) 
		{
			return (no != tidyOptIsReadOnly(opt));
		};

		/** Get category of given Option */
		TidyConfigCategory TidyOptGetCategory(TidyOption opt) 
		{
			return tidyOptGetCategory(opt);
		};

		/** Get default value of given Option as a String^ */
		String __gc * TidyOptGetDefault(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetDefault(opt);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
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
		String __gc * TidyOptGetNextPick(TidyOption opt, TidyIterator* pos) 
		{
			ctmbstr str = tidyOptGetNextPick(opt, pos);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Get current Option value as a String^ */
		String __gc * TidyOptGetValue(TidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetValue(tdoc, optId);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Set Option value as a String^ */
		bool TidyOptSetValue(TidyOptionId optId, String __gc * val) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(valstr, Encoding::UTF8->GetBytes(val));
			return (no != tidyOptSetValue(tdoc, optId, valstr));
		};

		/** Set named Option value as a string. Good if not sure of type. */
		bool TidyOptParseValue(String __gc * optnam, String __gc * val) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(optnamstr, Encoding::UTF8->GetBytes(optnam));
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(valstr, Encoding::UTF8->GetBytes(val));
			return (no != tidyOptParseValue(tdoc, optnamstr, valstr));
		};

		/** Get current Option value as an integer */
		ulong TidyOptGetInt(TidyOptionId optId) 
		{
			return tidyOptGetInt(tdoc, optId);
		};

		/** Set Option value as an integer */
		bool TidyOptSetInt(TidyOptionId optId, ulong val) 
		{
			return (no != tidyOptSetInt(tdoc, optId, val));
		};

		/** Get current Option value as a boolean flag */
		bool TidyOptGetBool(TidyOptionId optId) 
		{
			return (no != tidyOptGetBool(tdoc, optId));
		};

		/** Set Option value as a boolean flag */
		bool TidyOptSetBool(TidyOptionId optId, bool val) 
		{
			return (no != tidyOptSetBool(tdoc, optId, (val ? yes : no)));
		};

		/** Reset option to default value by ID */
		bool TidyOptResetToDefault(TidyOptionId opt) 
		{
			return (no != tidyOptResetToDefault(tdoc, opt));
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
		bool TidyOptCopyConfig(TidyParser __gc * To) 
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
		String __gc * TidyOptGetEncName(TidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetEncName(tdoc, optId);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Get current pick list value for option by ID. Useful for enum types. */
		String __gc * TidyOptGetCurrPick(TidyOptionId optId) 
		{
			ctmbstr str = tidyOptGetCurrPick(tdoc, optId);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Iterate over user declared tags */
		TidyIterator TidyOptGetDeclTagList(void) 
		{
			return tidyOptGetDeclTagList(tdoc);
		};

		/** Get next declared tag of specified type: TidyInlineTags, TidyBlockTags,
		** TidyEmptyTags, TidyPreTags */
		String __gc * TidyOptGetNextDeclTag(TidyOptionId optId, TidyIterator* iter) 
		{
			ctmbstr str = tidyOptGetNextDeclTag(tdoc, optId, iter);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
			return ret;
		};

		/** Get option description */
		String __gc * TidyOptGetDoc(TidyOption opt) 
		{
			ctmbstr str = tidyOptGetDoc(tdoc, opt);
			String __gc * ret = __gc new String(str, 0, tmbstrlen(str), Encoding::UTF8);
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
		int TidyParseFile(String __gc * filename) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Encoding::UTF8->GetBytes(filename));
			return tidyParseFile(tdoc, fname);
		};

		/** Parse markup from the standard input */
		int TidyParseStdin(void) 
		{
			return tidyParseStdin(tdoc);
		};

		/** Parse markup in given String^ */
		int TidyParseString(String __gc * content) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(str, Encoding::UTF8->GetBytes(content));
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
		int TidySaveFile(String __gc * filename) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Encoding::UTF8->GetBytes(filename));
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
		int TidySaveString([Out] String __gc *& buffer) // out buffer
		{
			TidyBuffer outbuf = {0};
			TidyOutputSink outsink = {0};
			tidyInitOutputBuffer(&outsink, &outbuf);
			int status = tidySaveSink(tdoc, &outsink);
			if (status >= 0)
			{
				String __gc * result = __gc new String((ctmbstr )outbuf.bp, 0, outbuf.size, Encoding::UTF8);
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
		int TidyOptSave(String __gc * dst) 
		{
			TidyBuffer outbuf = {0};
			int status = tidyOptSaveBuffer(tdoc, &outbuf);
			if (status >= 0)
			{
				String __gc * result = __gc new String((ctmbstr )outbuf.bp, 0, outbuf.size, Encoding::UTF8);
				dst = result;
			}
			tidyBufFree(&outbuf);
			return status;
		};

		int TidyOptSaveFile(String __gc * cfgfil) 
		{
			GENERATE_UNMANAGED_CMBTSTR_FOR_CURRENT_SCOPE(fname, Encoding::UTF8->GetBytes(cfgfil));
			return tidyOptSaveFile(tdoc, fname);
		};


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
