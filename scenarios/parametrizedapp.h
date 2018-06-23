#ifndef PARAMETRIZEDAPP_H
#define PARAMETRIZEDAPP_H

namespace ns3 {

struct ParametrizedApp : ndn::App
{
    ParametrizedApp(std::string p_ownPrefix, std::vector<std::string> p_interestedPrefixes = {})
        : m_ownPrefix(std::move(p_ownPrefix)),
          m_interestedPrefixes(std::move(p_interestedPrefixes))
    {}
    void StartApplication() override
    {
        ndn::App::StartApplication();
        ndn::FibHelper::AddRoute(GetNode(), m_ownPrefix, m_face, 0);
    }

    virtual void SendInterestImpl(const std::string& p_prefix, ndn::time::milliseconds p_time = ndn::time::seconds(1))
    {
        std::string interest_ctor_arg = p_prefix;
//        std::cout << "sent interest " << interest_ctor_arg << std::endl;
        auto interest = std::make_shared<ndn::Interest>(interest_ctor_arg);
        interest->setInterestLifetime(p_time);
        m_transmittedInterests(interest, this, m_face);
        m_appLink->onReceiveInterest(*interest);
    }
    void OnInterest(std::shared_ptr<const ndn::Interest> p_interest) override
    {
        ndn::App::OnInterest(p_interest);
//        std::cout << "on interest" << std::endl;
    }
    void OnData(std::shared_ptr<const ndn::Data> p_data) override
    {
        ndn::App::OnData(p_data);
//        std::cout << "on data" << std::endl;
    }
protected:
    int m_counter = 0;
    std::string m_ownPrefix;
    std::vector<std::string> m_interestedPrefixes;
};

}

#endif // PARAMETRIZEDAPP_H
