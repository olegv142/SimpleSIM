# SimpleSIM
The GSM module (SIM800) semi-asynchronous interface library for Arduino

The problem with GSM module interfacing lies in the fact that it used to send
asynchronous notifications in response of some events like SMS receiving. Typically
the interface code consists of the routines sending some API requests to the module
and expecting well defined response and the idle routine receiving asynchronous
notifications. Such approach is inherently unreliable. Consider what will happen in
case the asynchronous notification will be sent by the module nearly at the same time
we have chosen to send the API request to it. The request will fail since the response
will be considered invalid. The asynchronous notification will be lost as well since
it will be erroneously treated as API request response.

The SimpleSIM library follows semi-asynchronous design to copy with this problem
while having even less code than used by traditional approaches. The only responses
required for the API requests to complete are OK / ERROR results. Any other
request-specific result strings as well as asynchronous notifications are being captured
by special 'hooks' and placed to the hook's dedicated response buffer. The hooks are being
created and attached to the module interface by the user whenever necessary. Every hook
has prefix string matched against the lines received from the module. Whenever the prefix
matches the whole string will be placed to the hook buffer. The API user may subsequently
examine captured data.

See SMSEcho example for details.

## Author

Oleg Volkov (olegv142@gmail.com)
