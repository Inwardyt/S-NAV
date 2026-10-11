; Add this function to your game source.
; Call ExportSNavGrid("facility.snavmap") at the END of CreateMap(),
; after ConnectAdjacentRooms(). This exports the actual final room grid.
; Then import it in the simulator via Simulation > Import room grid.
Function ExportSNavGrid(path$)
 If MapWidth <> 18 Or MapHeight <> 18 Then Return False
 Local f% = WriteFile(path)
 If f = 0 Then Return False
 WriteLine f, "SNAV-MAP-1"
 Local x%, y%, row$
 For y = 0 To 18
  row = ""
  For x = 0 To 18
   If MapTemp(x, y) > 0 Then
    row = row + "#"
   Else
    row = row + "."
   EndIf
  Next
  WriteLine f, row
 Next
 CloseFile f
 Return True
End Function
