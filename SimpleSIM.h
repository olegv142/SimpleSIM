#pragma once

//
// The GSM module (SIM800) semi-asynchronous interface library
//

#include <Arduino.h> 
#include <string.h>

#define SIM_DEF_BOOT_DELAY 30000
#define SIM_DEF_CMD_TOUT   1000
#define SIM_DEF_MSG_TOUT   30000
#define SIM_RX_BUFF_SZ     128

// The hook is intended for capturing request responses and/or asynchronous notifications.
// It has prefix that will be matched against lines received from the module and the buffer
// where the captured content will be placed. Every time the prefix is matched the old
// buffer content (if any) will be replaced by the line received from the module.
class SIMHook {
friend class SimpleSIM;
public:
	SIMHook(const char* prefix)
		: m_prefix(prefix), m_prefix_len(strlen(prefix)), m_next(0)
		{}

	unsigned      captured() const { return m_buff.length(); }
	operator      bool()     const { return captured(); }
	String const& str()      const { return m_buff; }
	const char*   c_str()    const { return m_buff.c_str(); }
	void          reset()          { m_buff = String(); }

private:
	bool        hook(const char* buff, unsigned len);

	const char* m_prefix;
	unsigned    m_prefix_len;
	String      m_buff;
	SIMHook*    m_next;
};

// Result codes
typedef enum {
	sim_prompt =  1, // Prompt  (>)
	sim_ok     =  0, // Success (OK)
	sim_err    = -1, // Error   (ERROR)
	sim_tout   = -2  // Response time-out
} sim_result_t;

// The GSM module API
class SimpleSIM {
public:
	SimpleSIM(Stream& s, uint8_t rst_pin) :
		m_s(s), m_rst_pin(rst_pin), m_rst_ts(0),
		m_rx_len(0), m_hook_chain(0)
		{}

	// Add hook for capturing request responses and/or asynchronous notifications.
	// In case the message may be captured by several hooks the last added hook will
	// be chosen.
	void add_hook(SIMHook* hook) {
		hook->m_next = m_hook_chain;
		m_hook_chain = hook;
	}
	// Configure reset pin and issue initial reset to the module
	void begin();
	// Issue reset to the module
	void reset();

	// Wait the specified number of milliseconds receiving notifications if any.
	// Use this function in place of the delay in your code.
	void wait(unsigned msec);

	// Perform initial configuration of the module
	sim_result_t start(unsigned baud_rate, unsigned boot_delay = SIM_DEF_BOOT_DELAY);

	// Send command or continuation (e.g. SMS text)
	sim_result_t send(const char* cmd, unsigned tout, bool continuation);

	// Send command. The cmd will be prefixed by "AT" automatically. The command
	// will be terminated by <CR> automatically.
	sim_result_t send_cmd(const char* cmd, unsigned tout = SIM_DEF_CMD_TOUT) {
		return send(cmd, tout, false);
	}

	// Send message (e.g. SMS text). It will be terminated by Ctrl-Z automatically.
	sim_result_t send_msg(const char* cmd, unsigned tout = SIM_DEF_MSG_TOUT) {
		return send(cmd, tout, true);
	}

private:
	// Ensure the specified number of milliseconds elapsed since the last module reset
	void         wait_boot(unsigned msec);
	// Wait module response
	sim_result_t wait_resp(unsigned tout);

	Stream&	 m_s;

	uint8_t  m_rst_pin;
	uint32_t m_rst_ts;

	char     m_rx_buff[SIM_RX_BUFF_SZ+1];
	unsigned m_rx_len;

	SIMHook* m_hook_chain;
};

