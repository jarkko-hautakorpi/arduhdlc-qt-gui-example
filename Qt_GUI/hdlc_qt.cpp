#include "hdlc_qt.h"

HDLC_qt* HDLC_qt::m_Instance = 0;

/**
 * All data is passed to HDLC receiver.
 * Here we check the packet consistency, and pass on the data only.
 * @brief HDLC_qt::charReceiver
 * @param dataArray Received chunck of data of some length.
 * @return emit hdlcValidFrameReceived(data, length) When packet arrived.
 */
void HDLC_qt::charReceiver(QByteArray dataArray)
{
    quint8 data = 0;
    for(QByteArray::iterator it = dataArray.begin(); it != dataArray.end(); it++) {
        data = (*it);
        /* Start flag or end flag */
        if(data == FRAME_BOUNDARY_OCTET)
        {
            if(escape_character == true)
            {
                escape_character = false;
            }
            /* Do CRC check if frame is valid */
            else if(  (frame_position >= 2)
                      &&(this->hdlcFrameCRC_check(frame_position)) )
            {
                /* Call user defined function to handle HDLC frame */
                emit hdlcValidFrameReceived(receive_frame_buffer, (quint16)(frame_position-2));
            }
            /* Reset all for next frame */
            frame_position = 0;
            frame_checksum = 0;
            receive_frame_buffer.clear();
            continue;
        }

        if(escape_character)
        {
            escape_character = false;
            data ^= INVERT_OCTET;
        }
        else if(data == CONTROL_ESCAPE_OCTET)
        {
            escape_character = true;
            continue;
        }
        receive_frame_buffer[frame_position] = data;

        /* In Qt we don't calculate CRC here iteratively.
         * In Arduino, we needed to save RAM, but here small
         * buffer is no problem, we have more RAM.
         */

        frame_position++;

        /* If we don't ever receive valid frame,
         * buffer will keep growing bigger and bigger.
         * Hard coded max size limit and then reset
         */
        if(frame_position >= 2048)
        {
            receive_frame_buffer.clear();
            frame_position = 0;
            frame_checksum = 0;
        }
    }
}

/**
 * Wrap the data in HDLC frame
 * @brief HDLC_qt::frameDecode
 * @param buffer
 * @param bytes_to_send
 */
void HDLC_qt::frameDecode(QByteArray buffer, quint16 bytes_to_send)
{
    char data;
    QByteArray packet;
	/* The frame check sequence (FCS) is a 16-bit CRC-CCITT */
    quint16 fcs = 0;
    // Update checksum
    fcs = qChecksum((const char*)buffer.constData(), bytes_to_send);
	/* Start flag */
    packet.append((char)FRAME_BOUNDARY_OCTET);
    int i = 0;
    while (i < bytes_to_send)
    {
        data = buffer[i];
        if( (data == (char)CONTROL_ESCAPE_OCTET) || (data == (char)FRAME_BOUNDARY_OCTET) )
        {
            packet.append((char)CONTROL_ESCAPE_OCTET);
            data ^= (char)INVERT_OCTET;
        }
        packet.append((char)data);
        i++;
    }

    /* Invert bits in checksum
     * For avrlibc crc_ccitt_update() compatibility
	 */
    fcs ^= 0xFFFF;

    /* Low byte of inverted FCS */
    data = low(fcs);
    if((data == (char)CONTROL_ESCAPE_OCTET) || (data == FRAME_BOUNDARY_OCTET))
    {
        packet.append((char)CONTROL_ESCAPE_OCTET);
        data ^= (char)INVERT_OCTET;
    }
    packet.append((char)data);

    /* High byte of inverted FCS */
    data = high(fcs);
    if((data == (char)CONTROL_ESCAPE_OCTET) || (data == FRAME_BOUNDARY_OCTET))
    {
        packet.append((char)CONTROL_ESCAPE_OCTET);
        data ^= (char)INVERT_OCTET;
    }
    packet.append((char)data);

    /* End flag */
    packet.append((char)FRAME_BOUNDARY_OCTET);
    emit hdlcTransmitByte(packet);
}


bool HDLC_qt::hdlcFrameCRC_check(int frame_index)
{
    /* frame = ...[CRC-LO] [CRC-HI] */
    quint16 crc_received = 0;
    crc_received = receive_frame_buffer[frame_index-1]; // msb
    crc_received = crc_received << 8;
    crc_received |= receive_frame_buffer[frame_index-2]; // lsb
    quint16 crc_calculated = qChecksum((const char*)receive_frame_buffer.constData(), frame_index-2);
    crc_calculated = crc_calculated^0xFFFF;
    if(crc_received == crc_calculated) {
        return true;
    } else {
        return false;
    }
}

