#include "WavDraw.h"
#include "AssetController.h"
#include "Renderer.h"

WavDraw::WavDraw()
{
	m_header = { };
	m_data = nullptr;
	m_wSize = { };
	m_stepSize = 0;
	m_wavData = nullptr;
	m_xPos = 0;
	m_prevLeft = { };
	m_prevRight = { };
}

WavDraw::~WavDraw()
{
}

bool WavDraw::CompareFileExt(string _source, string _ext)
{
	transform(_source.begin(), _source.end(), _source.begin(), [](unsigned char c) {return tolower(c); });
	if (_source.length() >= _ext.length())
	{
		return (0 == _source.compare(_source.length() - _ext.length(), _ext.length(), _ext));
	}
	else
	{
		return false;
	}
}

void WavDraw::InterityChecks(Asset* _rawWav)
{
	// Check if we have any wave data
	M_ASSERT(_rawWav->GetDataSize() > 44, "Not a valid Wav fil");
	memcpy(&m_header, _rawWav->GetData(), sizeof(WAVHeader));

	// Check if format is supported
	string guid = _rawWav->GetGUID();
	M_ASSERT((CompareFileExt(guid, ".wav") == true), "Not a valid WAV file");
	int r = strcmp("RIFF", m_header.RIFFHeader);
	M_ASSERT(string(m_header.RIFFHeader, 4) == "RIFF", "Not a valid WAV file");
	M_ASSERT(string(m_header.FMTHeader, 3) == "fmt", "Not a valid WAV file");
	M_ASSERT(string(m_header.DataHeader, 4) == "data", "Not a valid WAV file");
	M_ASSERT((m_header.AudioFormat == 1), "Only supports PCM format");
	M_ASSERT((m_header.NumChannels == 2), "Can only display 2 channels");
	M_ASSERT((m_header.BitDepth == 16), "Can only display 16bps");
}

void WavDraw::DrawWave(Asset* _rawWav, Renderer* _renderer, float _yZoom)
{
	InterityChecks(_rawWav);

	// Render the axes
	m_wSize = _renderer->GetWindowSize();
	_renderer->SetDrawColor(Color(128, 128, 128, 255));
	_renderer->RenderLine(Rect(0, m_wSize.Y / 4, m_wSize.X, m_wSize.Y / 4));
	_renderer->RenderLine(Rect(0, m_wSize.Y * 3 / 4, m_wSize.X, m_wSize.Y * 3 / 4));

	// Render the WAV
	m_stepSize = m_header.Databytes / m_header.SampleAlignment / m_wSize.X * 2;
	m_wavData = (short*)(_rawWav->GetData() + 44);
	m_xPos = 0;
	for (int count = 0; count < m_header.Databytes; count += m_stepSize)
	{
		int leftYPos = *(m_wavData + count); // Left channel sample
		leftYPos = (int)(((m_wSize.Y / 4) + (float)leftYPos / m_wSize.Y * -1 * _yZoom));
		int rightYPos = *(m_wavData + count + 1); // Right channel sample
		rightYPos = (int)(((m_wSize.Y * 3 / 4) + (float)rightYPos / m_wSize.Y * -1 * _yZoom));
		if (count > 0)
		{
			_renderer->SetDrawColor(Color(255, 0, 0, 255));
			_renderer->RenderLine(Rect(m_xPos, leftYPos, m_prevLeft.X, m_prevLeft.Y));
			_renderer->SetDrawColor(Color(0, 0, 255, 255));
			_renderer->RenderLine(Rect(m_xPos, rightYPos, m_prevRight.X, m_prevRight.Y));
		}
		m_prevLeft = Point(m_xPos, leftYPos);
		m_prevRight = Point(m_xPos, rightYPos);
		m_xPos++;

		if (m_xPos > (int)m_wSize.X) break;

	}
}