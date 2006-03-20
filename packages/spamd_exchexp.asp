<%

dim server
server = "SERVERNAME"

Sub ExportUsers(oObject)
    Dim oUser
    For Each oUser in oObject
        Select Case oUser.Class
            Case "user"
                If oUser.mail <> "" then
              
                        for each email in oUser.proxyAddresses
                            If (lcase(left(email,4))="smtp") Then
                                'userFile.WriteLine Mid(email,6)
                                document.write Mid(email,6) & vbCrLf
                            End If
                        next
               End if
            Case "organizationalUnit" , "container"
                If UsersinOU (oUser) then
                    ExportUsers(oUser)
                End if
        End select
    Next
End Sub

Function UsersinOU (oObject)
    Dim oUser
    UsersinOU = False
    for Each oUser in oObject
        Select Case oUser.Class
            Case "organizationalUnit" , "container"
                UsersinOU = UsersinOU(oUser)
            Case "user"
             UsersinOU = True
            
        End select
    Next
End Function

Dim rootDSE, domainObject
Set rootDSE=GetObject("LDAP://" & server & "/RootDSE")
domainContainer = rootDSE.Get("defaultNamingContext")
Set domainObject = GetObject("LDAP://" & domainContainer)

ExportUsers(domainObject)
Set oDomain = Nothing

%>