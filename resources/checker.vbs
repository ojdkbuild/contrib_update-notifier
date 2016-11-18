Dim host
Dim fs
Dim path
Dim res

Set fs = CreateObject("Scripting.FileSystemObject")
Set path = fs.GetFile(WScript.ScriptFullName)
Set host = CreateObject("WScript.Shell")
res = host.Run(Chr(34) & path.ParentFolder & "\checker.exe" & Chr(34), 0, true)
WScript.Quit(res)