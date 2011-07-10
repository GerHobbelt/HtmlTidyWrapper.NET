/*
'
'    C# sample app for the HtmlTidyWrapper .NET wrapper for W3C tidy
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


using System;
using System.IO;
using System.Net;
using System.Text;
using System.Xml;
using System.Xml.Xsl;
using System.Xml.XPath;
using System.Security;
using System.Security.Permissions;
using System.Collections.Generic;

using HtmlTidy;


/*
 * derived from the sample code available at http://www.csharp-station.com/HowTo/HttpWebFetch.aspx
 * 
 * uses the HTMLTIDY library to clean up and format the grabbed HTML as XHTML/XML.
 */
namespace TidySampleCsharp
{
    class Program
    {
        static void Main(string[] args)
        {
            // used to build entire input
            StringBuilder sb = new StringBuilder();

            // used on each read operation
            byte[] buf = new byte[8192];

            // prepare the web page we will be asking for
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create("http://gathering.tweakers.net/forum/list_messages/1262310");

            // execute the request
            try
            {
                //throw new Exception("bogus");

                HttpWebResponse response = (HttpWebResponse)request.GetResponse();

                // we will read data via the response stream
                Stream resStream = response.GetResponseStream();

                string tempString = null;
                int count = 0;

                do
                {
                    // fill the buffer with data
                    count = resStream.Read(buf, 0, buf.Length);

                    // make sure we read some data
                    if (count != 0)
                    {
                        // translate from bytes to ASCII text
                        tempString = Encoding.ASCII.GetString(buf, 0, count);

                        // continue building the string
                        sb.Append(tempString);
                    }
                }
                while (count > 0); // any more data to read?
            }
            catch (WebException ex)
            {
                sb.AppendFormat("<html><body><h1>Error</h1><p>status {0} - {1}</html>",
                    ex.Status,
                    ex.Message);
            }
            catch (Exception ex)
            {
                sb.AppendFormat("<html><body><h1>Error</h1><p>generic error: {0}</html>",
                    ex.Message);
            }


            // now run the collected HTML through HTMLtidy to clean it and reformat as XHTML 
            // so we can access it using an XMLreader, for instance.
            TidyParser tidier = new TidyParser();

            bool cfg_ok = tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.ShowWarnings, true);
            cfg_ok &= tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.XhtmlOut, true);
            cfg_ok &= tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.XmlDecl, true);
            cfg_ok &= tidier.TidyOptSetAutoBool(HtmlTidy.HtmlTidyOptionId.IndentContent, HtmlTidyTriState.AutoState);

            cfg_ok &= tidier.TidyOptAdjustConfig();

            if (tidier.TidyConfigErrorCount() > 0)
            {
                Console.WriteLine("HTMLTIDY configuration error: {0}", tidier.ErrorMessage());

                tidier.TidyErrorSummary();

                Console.WriteLine("HTMLTIDY error summary: {0}", tidier.ErrorMessage());

                tidier.ResetErrorMessage();
            }

#if false
            String res = tidier.DoParseHtml2String(sb.ToString());

            Console.WriteLine("HTMLTIDY output\n{0}", res);
#endif

            XmlDocument xhtml_doc = tidier.DoParseHtml(sb.ToString());

            if (tidier.TidyErrorCount() > 0 || tidier.TidyWarningCount() > 0)
            {
                Console.WriteLine("HTMLTIDY HTML parsing errors: {0}", tidier.ErrorMessage());

                tidier.TidyErrorSummary();

                Console.WriteLine("HTMLTIDY HTML parsing error+warning summary: {0}", tidier.ErrorMessage());

                tidier.ResetErrorMessage();
            }

            Console.WriteLine("\nHTMLTIDY HTML parsing output:\n-----------------------------------\n");

            // print out reformatted XML/XHTML
            XmlWriterSettings settings = new XmlWriterSettings();
            settings.OmitXmlDeclaration = false;
            settings.Indent = true;
            settings.IndentChars = "  ";
            XmlWriter writer = XmlWriter.Create(Console.Out, settings); // deprecated: new XmlTextWriter(Console.Out);
            // writer.Formatting = Formatting.Indented; -- deprecated XmlTextWriter usage
            try
            {
                xhtml_doc.WriteTo(writer);
                writer.Flush();
            }
            catch (Exception ex)
            {
                writer.Flush();
                Console.WriteLine("XML output error: {0}\n", ex.Message);
                Console.WriteLine("XML output error source + stacktrace: {0}\n{1}\n", ex.Source, ex.StackTrace);
            }
            Console.WriteLine();

            // now try a search operation on the XmlDocument.
            XPathNavigator navigator = xhtml_doc.CreateNavigator();

            XPathExpression query = navigator.Compile("count(//html//br)");
            double total = (double)navigator.Evaluate(query);
            Console.WriteLine("Number of <br> elements = {0}\n", total);
            total = (double)navigator.Evaluate("count(//p)");
            Console.WriteLine("Number of <p> elements = {0}\n", total);
            
            // perform a bit of XSLT transformation on the XHTML:
#if false
            // Create the credentials.
            //WebPermission myWebPermission = new WebPermission(PermissionState.None);
            PermissionSet myPermissions = new PermissionSet(PermissionState.None); 
            //myPermissions.AddPermission(myWebPermission);

            XmlSecureResolver myResolver = new XmlSecureResolver(new XmlUrlResolver(), myPermissions);
            XmlReaderSettings mySettings = new XmlReaderSettings();
            mySettings.XmlResolver = myResolver;

            // Compile the style sheet.
            String xslt_str = "";
            TextReader xsl_txt_rdr = new StringReader(xslt_str);
            XmlTextReader xsl_reader = XmlReader.Create(xsl_txt_rdr, mySettings) as XmlTextReader;
            XslCompiledTransform xslt = new XslCompiledTransform(true);
            xslt.Load(xsl_reader, XsltSettings.Default, myResolver);	
#endif

        }
    }
}
