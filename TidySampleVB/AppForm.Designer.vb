<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class AppForm
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.GroupBox1 = New System.Windows.Forms.GroupBox
        Me.BrowseInputButton = New System.Windows.Forms.Button
        Me.InputFileText = New System.Windows.Forms.TextBox
        Me.Label1 = New System.Windows.Forms.Label
        Me.BrowseOutputButton = New System.Windows.Forms.Button
        Me.OutputFileText = New System.Windows.Forms.TextBox
        Me.Label2 = New System.Windows.Forms.Label
        Me.GroupBox2 = New System.Windows.Forms.GroupBox
        Me.GroupBox3 = New System.Windows.Forms.GroupBox
        Me.LogTextBox = New System.Windows.Forms.TextBox
        Me.RunTidyButton = New System.Windows.Forms.Button
        Me.GroupBox1.SuspendLayout()
        Me.GroupBox2.SuspendLayout()
        Me.GroupBox3.SuspendLayout()
        Me.SuspendLayout()
        '
        'GroupBox1
        '
        Me.GroupBox1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupBox1.Controls.Add(Me.BrowseInputButton)
        Me.GroupBox1.Controls.Add(Me.InputFileText)
        Me.GroupBox1.Controls.Add(Me.Label1)
        Me.GroupBox1.Location = New System.Drawing.Point(12, 12)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(692, 50)
        Me.GroupBox1.TabIndex = 0
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Input"
        '
        'BrowseInputButton
        '
        Me.BrowseInputButton.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.BrowseInputButton.Location = New System.Drawing.Point(603, 15)
        Me.BrowseInputButton.Name = "BrowseInputButton"
        Me.BrowseInputButton.Size = New System.Drawing.Size(75, 23)
        Me.BrowseInputButton.TabIndex = 2
        Me.BrowseInputButton.Text = "Browse"
        Me.BrowseInputButton.UseVisualStyleBackColor = True
        '
        'InputFileText
        '
        Me.InputFileText.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.InputFileText.Location = New System.Drawing.Point(67, 16)
        Me.InputFileText.Name = "InputFileText"
        Me.InputFileText.Size = New System.Drawing.Size(521, 20)
        Me.InputFileText.TabIndex = 1
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(7, 20)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(53, 13)
        Me.Label1.TabIndex = 0
        Me.Label1.Text = "Input File:"
        '
        'BrowseOutputButton
        '
        Me.BrowseOutputButton.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.BrowseOutputButton.Location = New System.Drawing.Point(603, 15)
        Me.BrowseOutputButton.Name = "BrowseOutputButton"
        Me.BrowseOutputButton.Size = New System.Drawing.Size(75, 23)
        Me.BrowseOutputButton.TabIndex = 2
        Me.BrowseOutputButton.Text = "Browse"
        Me.BrowseOutputButton.UseVisualStyleBackColor = True
        '
        'OutputFileText
        '
        Me.OutputFileText.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.OutputFileText.Location = New System.Drawing.Point(74, 16)
        Me.OutputFileText.Name = "OutputFileText"
        Me.OutputFileText.Size = New System.Drawing.Size(514, 20)
        Me.OutputFileText.TabIndex = 1
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(7, 20)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(61, 13)
        Me.Label2.TabIndex = 0
        Me.Label2.Text = "Output File:"
        '
        'GroupBox2
        '
        Me.GroupBox2.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupBox2.Controls.Add(Me.BrowseOutputButton)
        Me.GroupBox2.Controls.Add(Me.OutputFileText)
        Me.GroupBox2.Controls.Add(Me.Label2)
        Me.GroupBox2.Location = New System.Drawing.Point(12, 68)
        Me.GroupBox2.Name = "GroupBox2"
        Me.GroupBox2.Size = New System.Drawing.Size(692, 50)
        Me.GroupBox2.TabIndex = 1
        Me.GroupBox2.TabStop = False
        Me.GroupBox2.Text = "Output"
        '
        'GroupBox3
        '
        Me.GroupBox3.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupBox3.Controls.Add(Me.LogTextBox)
        Me.GroupBox3.Location = New System.Drawing.Point(12, 124)
        Me.GroupBox3.Name = "GroupBox3"
        Me.GroupBox3.Size = New System.Drawing.Size(692, 258)
        Me.GroupBox3.TabIndex = 3
        Me.GroupBox3.TabStop = False
        Me.GroupBox3.Text = "Logging"
        '
        'LogTextBox
        '
        Me.LogTextBox.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.LogTextBox.Location = New System.Drawing.Point(6, 19)
        Me.LogTextBox.MaxLength = 65536
        Me.LogTextBox.Multiline = True
        Me.LogTextBox.Name = "LogTextBox"
        Me.LogTextBox.ReadOnly = True
        Me.LogTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.LogTextBox.Size = New System.Drawing.Size(680, 233)
        Me.LogTextBox.TabIndex = 0
        '
        'RunTidyButton
        '
        Me.RunTidyButton.Anchor = CType(((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.RunTidyButton.Location = New System.Drawing.Point(12, 391)
        Me.RunTidyButton.Name = "RunTidyButton"
        Me.RunTidyButton.Size = New System.Drawing.Size(692, 23)
        Me.RunTidyButton.TabIndex = 4
        Me.RunTidyButton.Text = "Run Tidy/XML Process"
        Me.RunTidyButton.UseVisualStyleBackColor = True
        '
        'AppForm
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(716, 426)
        Me.Controls.Add(Me.RunTidyButton)
        Me.Controls.Add(Me.GroupBox3)
        Me.Controls.Add(Me.GroupBox2)
        Me.Controls.Add(Me.GroupBox1)
        Me.Name = "AppForm"
        Me.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show
        Me.Text = "HTMLtidy Sample"
        Me.GroupBox1.ResumeLayout(False)
        Me.GroupBox1.PerformLayout()
        Me.GroupBox2.ResumeLayout(False)
        Me.GroupBox2.PerformLayout()
        Me.GroupBox3.ResumeLayout(False)
        Me.GroupBox3.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents GroupBox1 As System.Windows.Forms.GroupBox
    Friend WithEvents BrowseInputButton As System.Windows.Forms.Button
    Friend WithEvents InputFileText As System.Windows.Forms.TextBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents BrowseOutputButton As System.Windows.Forms.Button
    Friend WithEvents OutputFileText As System.Windows.Forms.TextBox
    Friend WithEvents Label2 As System.Windows.Forms.Label
    Friend WithEvents GroupBox2 As System.Windows.Forms.GroupBox
    Friend WithEvents GroupBox3 As System.Windows.Forms.GroupBox
    Friend WithEvents LogTextBox As System.Windows.Forms.TextBox
    Friend WithEvents RunTidyButton As System.Windows.Forms.Button

    Public Sub New()

        ' This call is required by the Windows Form Designer.
        InitializeComponent()

        ' Add any initialization after the InitializeComponent() call.
        LogTextBox.Text = "This example application can load a HTML file from disk " & _
        "and reformat it as XHTML using the HtmlTidyWrapper and HTMLtidy " & _
        "library." & Chr(13) & Chr(10) & _
        "When you have selected an output XML file too, the result will be written " & _
        "to that file." & Chr(13) & Chr(10) & _
        "Anyhow, the input content, transformation messages and result are also written " & _
        "to this log window for inspection." & Chr(13) & Chr(10) & _
        "When you are looking for an example which produce an XmlDocument, which can " & _
        "be searched and/or otherwise processed, see the C# command line " & _
        "example instead." & Chr(13) & Chr(10)

    End Sub
End Class
