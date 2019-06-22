# SimpleSIM
The GSM module (SIM800) semi-asynchronous interface library for Arduino

The problem with GSM module interfacing lies in the fact that it used to send
asynchronous notifications in response to some events like SMS receiving. Typically
the interface code consists of the routines sending some API requests to the module
and expecting well defined response and the idle routine receiving asynchronous
notifications. Such approach is inherently unreliable. Consider what will happen in
case the asynchronous notification will be sent by the module nearly at the same time
we have chosen to send the API request to it. The request will fail since the response
will be considered invalid. The asynchronous notification will be lost as well since
it will be erroneously treated as API request response.

The SimpleSIM library follows semi-asynchronous design to cope with this problem
while having even less code than used by traditional approaches. The only responses
required for the API requests to complete are OK / ERROR results. Any other
request-specific result strings as well as asynchronous notifications are being captured
by special 'hooks' and placed to the hook's dedicated response buffers. The hooks are being
created and attached to the interface instance by the user whenever necessary. Every hook
has prefix string matched against the lines received from the module. Whenever the prefix
matches the whole string will be placed to the hook buffer. The API user may subsequently
examine captured data.

The following figure shows how to use hook to query signal quality information.

![Using hook for capturing signal quality report](https://github.com/olegv142/SimpleSIM/blob/master/doc/SimpleSIM.png)

To query signal quality information the code calls send_cmd("+CSQ") on the SimpleSIM class instance.
The method implementation adds "AT" prefix and send resultant string to the GSM module. The module
respond with signal quality report "+CSQ: ..." followed by completion status "OK" on success or "ERROR"
on failure. The SimpleSIM class instance does not handle signal quality report by itself. It just wait for
the completion status reception and returns to the caller. All other strings received from GSM module are
handled by SIMHook class instances. Should the string received matches the m_prefix field of the hook the
entire string would be stored in the hook's m_buff field. There are several methods to access that buffer.
The captured() method returns the number of characters in the buffer. The str() and c_str() return content
as string or character array pointer. The reset() method erases buffer content. Any number of hooks may be
attached to the SimpleSIM class instance to capture different report strings each with its own prefix.

See SMSEcho for more elaborated example showing how to receive and send SMS messages.

## Author

Oleg Volkov (olegv142@gmail.com)
