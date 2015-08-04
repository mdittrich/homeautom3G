/*!  \file    netlib.cpp
     \brief   Net library to communicate throught www.
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

#include "netlib.h"

/*!
	\brief      Constructor of the class serialib.
*/
netlib::netlib()
{}

/*!
	\brief      Destructor of the class serialib. It close the connection
*/
netlib::~netlib()
{
	exit(0);
}

/*!
	\brief      Callback function writes data to a std::ostream
*/
size_t netlib::data_write(void *buf, size_t size, size_t nmemb, void *userp) {
	if(userp) {
		std::ostream &os = *static_cast<std::ostream*>(userp);
		std::streamsize len = size *nmemb;
		if(os.write(static_cast<char*>(buf), len))
			return len;
	}
	return 0;
}

/*!
	\brief      Get HTML response
*/
CURLcode netlib::curl_read(const std::string &url, std::ostream &os, long timeout = 10) {
	// Timeout is in seconds
	CURLcode code(CURLE_FAILED_INIT);
	CURL *curl = curl_easy_init();
	if(curl) {
		if((code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write)) == CURLE_OK
		and (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L)) == CURLE_OK
		and (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)) == CURLE_OK
		and (code = curl_easy_setopt(curl, CURLOPT_FILE, &os)) == CURLE_OK
		and (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout)) == CURLE_OK
		and (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())) == CURLE_OK) {
		// then do
			code = curl_easy_perform(curl);
		}
		curl_easy_cleanup(curl);
	}
	return code;
}

/*!
	\brief      Transforms oString-stream to char*
*/
char* netlib::getData(const char *url) {
	std::ostringstream oss;
	char *ip = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	if(curl_read(url, oss) == CURLE_OK) {
		// Body successfully written to String
		ip = (char*)oss.str().c_str();
	}
	curl_global_cleanup();
	return ip;
}

