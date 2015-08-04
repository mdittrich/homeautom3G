/*!  \file    netlib.h
     \brief   Class to manage the requests.
     \author  Martin Dittrich (University of Hamburg) <martin.dittrich@tuhh.de>
     \version 1.0
     \date    28 june 2015

     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
     INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
     PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM,
     DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
     FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

     This is a licence-free software, it can be used by anyone who try to build a better world.
*/

#ifndef NETLIB_H
#define NETLIB_H

// Used for Curl operations
#include <curl/curl.h>

// Include for Linux
#ifdef __linux__
    #include <stdlib.h>
    #include <string.h>
	#include <fstream>
	#include <sstream>
#endif

/*!  \class netlib
     \brief     This class can manage a www get request.
     \example   Example1.cpp
   */

class netlib
{
public:
    // Constructor of the class
    netlib    ();

    // Destructor
    ~netlib   ();

    //_________________________________________
    // ::: Read/Write operations :::

    // Read a stream into os
	CURLcode	curl_read	(	const std::string &url,
								std::ostream &os,
								long timeout);

    // Callback
	static	size_t	data_write	(	void *buf,
									size_t size,
									size_t nmemb,
									void *userp);

    //___________________________________________
    // ::: Returns characters :::

    // Read a char*
    char*	getData	(const char *url);
};

#endif // NETLIB_H

