### Running the Program

- make
- ./fuzzer http://example.com search
- python -m http.server

<pre>
MacBook-Air-2:web-fuzzer anish$ python -m http.server
Serving HTTP on :: port 8000 (http://[::]:8000/) ...
::ffff:127.0.0.1 - - [14/Feb/2025 20:31:56] "GET /?search=%27%20OR%20%271%27%3D%271 HTTP/1.1" 200 -
::ffff:127.0.0.1 - - [14/Feb/2025 20:31:56] "GET /?search=..%2F..%2Fetc%2Fpasswd HTTP/1.1" 200 -
::ffff:127.0.0.1 - - [14/Feb/2025 20:31:56] "GET /?search=admin%27%20-- HTTP/1.1" 200 -
::ffff:127.0.0.1 - - [14/Feb/2025 20:31:56] "GET /?search=%3Cscript%3Ealert%28%27XSS%27%29%3C%2Fscript%3E HTTP/1.1" 200 -
::ffff:127.0.0.1 - - [14/Feb/2025 20:31:56] "GET /?search=%2500 HTTP/1.1" 200 -
</pre>


<pre>
MacBook-Air-2:web-fuzzer anish$ ./fuzzer http://127.0.0.1:8000 search
[+] Testing: http://127.0.0.1:8000?search=%27%20OR%20%271%27%3D%271
[+] Testing: http://127.0.0.1:8000?search=..%2F..%2Fetc%2Fpasswd
[+] Testing: [+] Testing: http://127.0.0.1:8000?search=%3Cscript%3Ealert%28%27XSS%27%29%3C%2Fscript%3E
[+] Testing: http://127.0.0.1:8000?search=%2500
http://127.0.0.1:8000?search=admin%27%20--
...
...
...
</pre>
