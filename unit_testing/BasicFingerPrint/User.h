#ifndef _HELPER_H_
#define _HELPER_H_

#include <iostream>
#include <string>
#include <TimeLib.h> 


using std::string;

class User {
private:
	string id;              // 9 digits
	uint8_t fingerprint_id; // finger print id 
	bool is_approved;       
  bool is_appending;
  time_t current_time;

public:
	User() : id(), fingerprint_id(), is_approved(false), current_time() {}
	User(const User& user) = default;
	User& operator=(const User&) = default;
	~User() = default;

	void setUserAproovedOn() {
		is_approved = true;
	}

	void setUserAproovedOff() {
		is_approved = false;
	}

  	void setUserAppendingOn() {
		is_appending = true;
	}

	void setUserAppendingOff() {
		is_appending = false;
	}

  void setFingerPrintID(uint8_t finger_id) {
		fingerprint_id = fingerprint_id;
	}

  void setID(string user_id) {
    id = user_id;
  }

  string getID() {
    return id;
  }

  bool isUserApproved() {
    return is_approved;
  }

  bool isUserAppending() {
    return is_appending;
  }

  uint8_t getFingerprintID() {
    return fingerprint_id;
  }


  void setCurrentTime() {
    current_time = now(); 
  }

  time_t getCurrentTime() {
    return current_time;
  }
};

#endif /* _HELPER_H_ */
