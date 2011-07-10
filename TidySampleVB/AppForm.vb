'
'    VB.NET sample app for the HtmlTidyWrapper .NET wrapper for W3C tidy
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

Public Class AppForm

    Protected input_directory As String = "p:\"
    Protected output_directory As String = "c:\"

    Private Sub BrowseInputButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles BrowseInputButton.Click
        Dim openFileDialog1 As New OpenFileDialog()

        openFileDialog1.InitialDirectory = input_directory
        openFileDialog1.Filter = "HTML files (*.html)|*.htm*|XML files (*.xml)|*.xml|All files (*.*)|*.*"
        openFileDialog1.FilterIndex = 1
        openFileDialog1.RestoreDirectory = False
        openFileDialog1.CheckFileExists = True
        openFileDialog1.CheckPathExists = True
        openFileDialog1.Multiselect = False
        openFileDialog1.ReadOnlyChecked = True

        If openFileDialog1.ShowDialog() = System.Windows.Forms.DialogResult.OK Then
            Dim path As String = openFileDialog1.FileName()
            InputFileText.Text = path

            Dim testFile As System.IO.FileInfo
            testFile = My.Computer.FileSystem.GetFileInfo(openFileDialog1.FileName())
            Dim folderPath As String = testFile.DirectoryName
            input_directory = folderPath
        End If
    End Sub

    Private Sub BrowseOutputButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles BrowseOutputButton.Click
        Dim openFileDialog1 As New OpenFileDialog()

        openFileDialog1.InitialDirectory = output_directory
        openFileDialog1.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*"
        openFileDialog1.FilterIndex = 1
        openFileDialog1.RestoreDirectory = False
        openFileDialog1.CheckFileExists = False
        openFileDialog1.CheckPathExists = True
        openFileDialog1.Multiselect = False
        openFileDialog1.ReadOnlyChecked = False

        If openFileDialog1.ShowDialog() = System.Windows.Forms.DialogResult.OK Then
            Dim path As String = openFileDialog1.FileName()
            OutputFileText.Text = path

            Dim testFile As System.IO.FileInfo
            testFile = My.Computer.FileSystem.GetFileInfo(openFileDialog1.FileName())
            Dim folderPath As String = testFile.DirectoryName
            output_directory = folderPath
        End If

    End Sub

    Private Sub RunTidyButton_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles RunTidyButton.Click
        Dim fileReader As System.IO.StreamReader
        Try
            fileReader = My.Computer.FileSystem.OpenTextFileReader(InputFileText.Text)
            Dim stringReader As String
            stringReader = fileReader.ReadToEnd()
            fileReader.Close()

            LogTextBox.WordWrap = True
            LogTextBox.Text = "File read:" & Chr(13) & Chr(10)
            LogTextBox.Text = LogTextBox.Text & stringReader & Chr(13) & Chr(10)

            Dim tidier As HtmlTidy.TidyParser = New HtmlTidy.TidyParser()

            Dim cfg_ok As Boolean = tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.ShowWarnings, True)
            cfg_ok = cfg_ok And tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.XhtmlOut, True)
            cfg_ok = cfg_ok And tidier.TidyOptSetBool(HtmlTidy.HtmlTidyOptionId.XmlDecl, True)
            cfg_ok = cfg_ok And tidier.TidyOptSetInt(HtmlTidy.HtmlTidyOptionId.WrapLen, 160)
            cfg_ok = cfg_ok And tidier.TidyOptSetAutoBool(HtmlTidy.HtmlTidyOptionId.IndentContent, HtmlTidy.HtmlTidyTriState.AutoState)

            cfg_ok = cfg_ok And tidier.TidyOptAdjustConfig()

            LogTextBox.Text = LogTextBox.Text & Chr(13) & Chr(10) & "---------------- HTMLTIDY -------------------:" & Chr(13) & Chr(10)
            If tidier.TidyConfigErrorCount() > 0 Then
                LogTextBox.Text = LogTextBox.Text & "HTMLTIDY configuration error: " & tidier.ErrorMessage() & Chr(13) & Chr(10)

                tidier.TidyErrorSummary()

                LogTextBox.Text = LogTextBox.Text & "HTMLTIDY error summary: " & tidier.ErrorMessage() & Chr(13) & Chr(10)

                tidier.ResetErrorMessage()
            End If


            Dim fmt_str As String = tidier.DoParseHtml2String(stringReader)

            LogTextBox.Text = LogTextBox.Text & "HTMLTIDY File formatted:" & Chr(13) & Chr(10)
            LogTextBox.Text = LogTextBox.Text & fmt_str

            If OutputFileText.Text.Length > 0 Then

                Dim fileWr As System.IO.StreamWriter

                Try
                    fileWr = My.Computer.FileSystem.OpenTextFileWriter(OutputFileText.Text, False)
                    fileWr.Write(fmt_str)
                    fileWr.Close()

                Catch Ex As Exception
                    MessageBox.Show("Cannot write file '" & OutputFileText.Text & "'. Original error: " & Ex.Message)

                Finally

                    ' nada
                End Try

            End If

        Catch Ex As Exception
            MessageBox.Show("Cannot read file '" & InputFileText.Text & "' from disk or format through HTMLtidy. Original error: " & Ex.Message)

        Finally

            ' nada
        End Try
    End Sub


End Class
