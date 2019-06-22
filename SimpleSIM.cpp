//
// The GSM module (SIM800) semi-asynchronous interface module
//

#include "SimpleSIM.h"

// Configure reset pin
void SimpleSIM::begin()
{
	digitalWrite(m_rst_pin, HIGH);
	pinMode(m_rst_pin, OUTPUT);
	m_rst_ts = millis();
}

// Issue reset to the module
void SimpleSIM::reset()
{
	digitalWrite(m_rst_pin, LOW);
	delay(100);
	digitalWrite(m_rst_pin, HIGH);
	m_rst_ts = millis();
}

// Ensure the specified number of milliseconds elapsed since the last module reset
void SimpleSIM::wait_boot(unsigned msec)
{
	uint32_t now = millis();
	uint32_t uptime = now - m_rst_ts;
	if (uptime < msec)
		wait(msec - uptime);
}

// Perform initial configuration of the module
sim_result_t SimpleSIM::start(unsigned baud_rate, unsigned boot_delay)
{
	sim_result_t res;

	wait_boot(boot_delay);

	String set_rate_cmd("+IPR=");
	set_rate_cmd += baud_rate;

	// Auto detect baud rate
	if ((res = send_cmd(0)) != sim_ok)
		return res;
	// Configure baud rate
	if ((res = send_cmd(set_rate_cmd.c_str())) != sim_ok)
		return res;
	// Set text mode for SMS
	if ((res = send_cmd("+CMGF=1")) != sim_ok)
		return res;
	// Route SMS to serial
	if ((res = send_cmd("+CNMI=1,2,0,0,0")) != sim_ok)
		return res;
	// No verbose error info
	if ((res = send_cmd("+CMEE=0")) != sim_ok)
		return res;
	// Echo off
	if ((res = send_cmd("E0")) != sim_ok)
		return res;
	// Save configuration
	if ((res = send_cmd("&W")) != sim_ok)
		return res;

	return sim_ok;
}

// Send command or continuation (e.g. SMS text)
sim_result_t SimpleSIM::send(
		const char* cmd,
		unsigned tout,
		bool continuation
	)
{
	if (!continuation) {
		m_s.print("AT");
	}
	if (cmd) {
		m_s.print(cmd);
	}
	m_s.write(continuation ? 26 : '\r');
	return wait_resp(tout);
}

// Wait module response
sim_result_t SimpleSIM::wait_resp(unsigned tout)
{
	uint32_t start = millis();
	for (;;) {
		if (m_s.available()) {
			int c = m_s.read();
			if (c == '\n' && m_rx_len > 0 && m_rx_buff[m_rx_len-1] == '\r') {
				// End of the line received
				if (
					m_rx_len == 3 &&
					m_rx_buff[0] == 'O' && m_rx_buff[1] == 'K'
				) {
					m_rx_len = 0;
					return sim_ok;
				}
				if (
					m_rx_len == 6 &&
					m_rx_buff[0] == 'E' && m_rx_buff[1] == 'R' && m_rx_buff[2] == 'R' && m_rx_buff[3] == 'O' && m_rx_buff[4] == 'R'
				) {
					m_rx_len = 0;
					return sim_err;
				}
				m_rx_buff[m_rx_len-1] = 0;
				for (SIMHook* h = m_hook_chain; h; h = h->m_next) {
					if (h->hook(m_rx_buff, m_rx_len-1))
						break;
				}
				m_rx_len = 0;
			} else {
				m_rx_buff[m_rx_len] = c;
				if (m_rx_len < SIM_RX_BUFF_SZ)
					++m_rx_len;
			}
		} else if (millis() - start > tout) {
			if (
				m_rx_len == 2 &&
				m_rx_buff[0] == '>' && m_rx_buff[1] == ' '
			) {
				m_rx_len = 0;
				return sim_prompt;
			}
			return sim_tout;
		}
	}
}

// Wait the specified number of milliseconds receiving notifications if any.
// Use this function in place of the delay in your code.
void SimpleSIM::wait(unsigned msec)
{
	uint32_t start = millis(), tout = msec;
	for (;;) {
		if (wait_resp(tout) == sim_tout)
			return;
		uint32_t elapsed = millis() - start;
		if (elapsed >= msec)
			break;
		tout = msec - elapsed;
	}
}

// The hook is intended for capturing request responses and/or asynchronous notifications.
// It has prefix that will be matched against lines received from the module and the buffer
// where the captured content will be placed. Every time the prefix is matched the old
// buffer content (if any) will be replaced by the line received from the module.
bool SIMHook::hook(const char* buff, unsigned len)
{
	if (len >= m_prefix_len && !memcmp(buff, m_prefix, m_prefix_len)) {
		m_buff = buff;
		return true;
	}
	return false;
}
