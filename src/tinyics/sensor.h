#pragma once

class AnalogSensor
{
public:
    AnalogSensor() = default;

    AnalogSensor(double min, double max) : m_Min(min), m_Max(max), m_Value(0) {}

    AnalogSensor(double min, double max, double value) : m_Min(min), m_Max(max), m_Value(value) {}

    /*
     * Copy assignment.
     *
     * Lets us do sensor_obj = 3.0 with an initialized sensor object;
     */
    AnalogSensor& operator=(double value)
    {
        m_Value = value;
        return *this;
    }

    AnalogSensor& operator++()
    {
        m_Value++;
        return *this;
    }

    AnalogSensor& operator--()
    {
        m_Value--;
        return *this;
    }

    AnalogSensor& operator+=(double val)
    {
        m_Value += val;
        return *this;
    }

    AnalogSensor& operator-=(double val)
    {
        m_Value -= val;
        return *this;
    }

    AnalogSensor& operator*=(double val)
    {
        m_Value *= val;
        return *this;
    }

    /*
     * Comparison operators for double
     */
    bool operator==(const double& b) const
    {
        return m_Value == b;
    }

    bool operator!=(const double& b) const
    {
        return m_Value != b;
    }

    bool operator<(const double& b) const
    {
        return m_Value < b;
    }

    bool operator>(const double& b) const
    {
        return m_Value > b;
    }

    bool operator<=(const double& b) const
    {
        return m_Value <= b;
    }

    bool operator>=(const double& b) const
    {
        return m_Value >= b;
    }

    /*
     * Returns the value of the measured input in the range 4-20mA
     */
    double GetOutputValue() const
    {
        // (20mA - 4mA) / (max - min)
        auto scalingFactor = 16 / (m_Max - m_Min);
        auto current = ((m_Value - m_Min) * scalingFactor) + 4;

        // Clamp the values in the range 4-20
        if (current > 20)
            return 20.0;

        if (current < 4)
            return 4.0;

        return current;

    }

    double GetValue() const
    {
        return m_Value;
    }

    /*
     * Set the measured value, the original value measured by the sensor (could
     * represent temperature, presion, etc.)
     */
    void SetValue(double value)
    {
        m_Value = value;
    }

private:
    double m_Value;
    double m_Min;
    double m_Max;
};

