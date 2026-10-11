; Add this function to the game source and call it at the end of CreateMap(),
; after ConnectAdjacentRooms(): ExportSNavGrid("facility.snavmap")
; Import the resulting file with Simulation > Import room grid.
Function ExportSNavGrid(path$)
 If MapWidth <> 18 Or MapHeight <> 18 Then Return False
 Local f% = WriteFile(path)
 If f = 0 Then Return False
 WriteLine f, "SNAV-MAP-2"
 Local x%, y%, row$, r.Rooms
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
 For r = Each Rooms
  x = Floor(r\x / 8.0 + 0.5)
  y = Floor(r\z / 8.0 + 0.5)
  If x >= 0 And x <= 18 And y >= 0 And y <= 18 And r\y = 0.0 Then
   If MapTemp(x, y) > 0 Then WriteLine f, x + " " + y + " " + r\RoomTemplate\Name
  EndIf
 Next
 CloseFile f
 Return True
End Function
