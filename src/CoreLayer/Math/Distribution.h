#pragma once
#include <algorithm>
#include <functional>
#include <vector>

template <typename T>
class Distribution {
  public:
    Distribution() = default;

    Distribution(const std::vector<T> &data,
                 const std::function<float(T)> &weightFunction)
        : _data(data) {
        _cdf.reserve(data.size() + 1);
        _cdf.emplace_back(.0f);

        for (int i = 0; i < data.size(); ++i) {
            float weight = weightFunction(data[i]);
            _cdf.emplace_back(weight + _cdf.back());
        }

        float invTotal = 1.f / _cdf.back();
        for (int i = 0; i < _cdf.size(); ++i) {
            _cdf[i] *= invTotal;
        }
    }

    T sample(float sample, float *pdf) const {
        if (_cdf.size() == 1) {
            // no data in distribution
            *pdf = .0f;
            return T();
        }
        auto entry = std::lower_bound(_cdf.cbegin(), _cdf.cend(), sample);
        size_t index = entry - _cdf.cbegin() - 1;
        *pdf = _cdf[index + 1] - _cdf[index];
        return _data[std::min(index, _cdf.size() - 2)];
    }

    float pdf(T sampled) const {
        auto entry = std::find(_data.cbegin(), _data.cend(), sampled);
        if (entry == _data.cend())
            return .0f;
        size_t index = entry - _data.cbegin();
        return _cdf[index + 1] - _cdf[index];
    }

  private:
    std::vector<float> _cdf;
    std::vector<T> _data;
};
