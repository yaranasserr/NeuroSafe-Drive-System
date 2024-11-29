#include <torch/torch.h>
#include <Eigen/Dense>
#include <cmath>
#include <vector>

// Utility function to perform normal sampling
double normal_sampling(double mean, double label_k, double std) {
    return std::exp(-(label_k - mean) * (label_k - mean) / (2 * std * std)) / (std::sqrt(2 * M_PI) * std);
}

// KL divergence loss function
double kl_loss(const torch::Tensor& inputs, const torch::Tensor& labels) {
    auto outputs = inputs.log();
    auto loss = torch::kl_div(outputs, labels, torch::Reduction::Sum);
    return loss.item<double>();
}

class TorchLossComputer {
public:
    static torch::Tensor compute_complex_absolute_given_k(const torch::Tensor& output, const torch::Tensor& k, int N) {
        auto two_pi_n_over_N = torch::linspace(0, 2 * M_PI, N) / N;
        auto hanning = torch::from_blob(np::hanning(N).data(), {N}, torch::kFloat32).view({1, -1});
        auto output_reshaped = output.view({1, -1}) * hanning;
        output_reshaped = output_reshaped.view({1, 1, -1});
        auto k_reshaped = k.view({1, -1, 1});
        auto two_pi_n_over_N_reshaped = two_pi_n_over_N.view({1, 1, -1});
        auto complex_absolute = torch::sum(output_reshaped * torch::sin(k_reshaped * two_pi_n_over_N_reshaped), -1).pow(2) +
                                torch::sum(output_reshaped * torch::cos(k_reshaped * two_pi_n_over_N_reshaped), -1).pow(2);
        return complex_absolute;
    }

    static torch::Tensor complex_absolute(const torch::Tensor& output, double Fs, const torch::Tensor& bpm_range) {
        auto N = output.size(1);
        double unit_per_hz = Fs / N;
        auto feasible_bpm = bpm_range / 60.0;
        auto k = feasible_bpm / unit_per_hz;
        auto complex_absolute = compute_complex_absolute_given_k(output, k, N);
        return (1.0 / complex_absolute.sum()) * complex_absolute;
    }

    static std::pair<torch::Tensor, torch::Tensor> cross_entropy_power_spectrum_loss(const torch::Tensor& inputs, const torch::Tensor& target, double Fs) {
        auto bpm_range = torch::arange(40, 180, torch::kFloat);
        auto complex_absolute = complex_absolute(inputs, Fs, bpm_range);
        auto max_val = complex_absolute.view(-1).max(0);
        auto max_idx = max_val.index().to(torch::kFloat);
        return {torch::cross_entropy(complex_absolute, target.view({1}).to(torch::kLong)), torch::abs(target[0] - max_idx)};
    }

    static std::pair<torch::Tensor, torch::Tensor> cross_entropy_power_spectrum_focal_loss(const torch::Tensor& inputs, const torch::Tensor& target, double Fs, double gamma) {
        auto bpm_range = torch::arange(40, 180, torch::kFloat);
        auto complex_absolute = complex_absolute(inputs, Fs, bpm_range);
        auto max_val = complex_absolute.view(-1).max(0);
        auto max_idx = max_val.index().to(torch::kFloat);
        auto criterion = FocalLoss(gamma);
        return {criterion(complex_absolute, target.view({1}).to(torch::kLong)), torch::abs(target[0] - max_idx)};
    }

    static torch::Tensor cross_entropy_power_spectrum_forward_pred(const torch::Tensor& inputs, double Fs) {
        auto bpm_range = torch::arange(40, 190, torch::kFloat);
        auto complex_absolute = complex_absolute(inputs, Fs, bpm_range);
        auto max_val = complex_absolute.view(-1).max(0);
        return max_val.index().to(torch::kFloat);
    }

    static std::tuple<double, torch::Tensor, torch::Tensor> cross_entropy_power_spectrum_DLDL_softmax2(const torch::Tensor& inputs, const torch::Tensor& target, double Fs, double std) {
        std::vector<double> target_distribution;
        for (int i = 0; i < 140; ++i) {
            double val = normal_sampling(target.item<double>(), i, std);
            target_distribution.push_back(val > 1e-15 ? val : 1e-15);
        }
        auto target_distribution_tensor = torch::tensor(target_distribution, torch::kFloat);
        auto rank = torch::arange(140, torch::kFloat);
        auto bpm_range = torch::arange(40, 180, torch::kFloat);
        auto complex_absolute = complex_absolute(inputs, Fs, bpm_range);
        auto fre_distribution = torch::softmax(complex_absolute.view(-1), 0);
        auto loss_distribution_kl = kl_loss(fre_distribution, target_distribution_tensor);
        auto max_val = complex_absolute.view(-1).max(0);
        auto max_idx = max_val.index().to(torch::kFloat);
        return {loss_distribution_kl, torch::cross_entropy(complex_absolute, target.view({1}).to(torch::kLong)), torch::abs(target[0] - max_idx)};
    }
};
