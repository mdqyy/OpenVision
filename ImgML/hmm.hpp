#ifndef HMM_H
#define HMM_H

#include "ImgML/eigenutils.hpp"
#include "Common/OpenCVCommon.hpp"

/**
 * @brief The HMM class :
 * This class encapsulates the representation of a Discrete Hidden Markov model
 * A hidden markov model is composed of transition matrix,emission matrix
 * and initial probability matrix
 */
class HMM
{
public:
    MatrixXf _transition;
    MatrixXf _initial;
    MatrixXf _emission;
    int _nstates;
    int _nobs;
    int _seqlen;
    MatrixXf _alpha;
    MatrixXf _beta;
    MatrixXf _scale;

    /**
     * @brief setData : method to set the parameters of the model
     * @param transition
     * @param emission
     * @param initial
     * @param seqlen  : the maximum sequence length of sequence,for
     *                  allocating matrices
     */
    void setData(Mat transition,Mat emission,Mat initial,int seqlen=30)
    {

        _transition=EigenUtils::setData(transition);
        _initial=EigenUtils::setData(initial);
        _emission=EigenUtils::setData(emission);
        _nstates=_transition.rows();
        _nobs=_emission.rows();
        _seqlen=seqlen;

        _alpha=MatrixXf(_nstates,_seqlen+1);
        _scale=MatrixXf(1,_seqlen+1);

    }


    //computing p(xn.....xN,zn)
    void backwardMatrix(vector<int> &sequence)
    {
        _beta=MatrixXf(_nstates,sequence.size()+1);
        int len=sequence.size()+1;
        for(int i=len-1;i>=0;i--)
        {
            for(int j=0;j<_nstates;j++)
            {
                if(i==len-1)
                {
                    _beta(j,i)=1;
                }
                else
                {
                    float s=0;
                    for(int k=0;k<_nstates;k++)
                        s=s+_beta(k,i+1)*_emission(k,sequence[i])*_transition(j,k);
                    _beta(j,i)=s*_scale(0,i+1);

                }


            }

        }
    }

    /**
     * @brief forwardMatrix : method computes probability //compute p(x1 ... xn,zn)
     *                        using the forward algorithm
     * @param sequence : is input observation sequence
     */
    void forwardMatrix(vector<int> &sequence)
    {
        int len=sequence.size()+1;
        for(int i=0;i<len;i++)
        {
        for(int j=0;j<_nstates;j++)
        {
            if(i==0)
                _alpha(j,i)=_emission(j,sequence[i])*_initial(0,j);
            else
            {
                float s=0;
                for(int k=0;k<_nstates;k++)
                s=s+_transition(k,j)*_alpha(k,i-1);
                _alpha(j,i)=_emission(j,sequence[i-1])*s;
            }

        }
        float scale=0;
        for(int j=0;j<_nstates;j++)
        {
            scale=scale+_alpha(j,i);
        }
        scale=1.f/scale;
        if(i==0)
            _scale(0,i)=1;
        else
            _scale(0,i)=scale;

        for(int j=0;j<_nstates;j++)
        {
            _alpha(j,i)=scale*_alpha(j,i);

        }


        }


    }


    void forwardMatrixI(vector<int> &sequence)
    {
        int len=sequence.size()+1;
        for(int i=0;i<len;i++)
        {
        for(int j=0;j<_nstates;j++)
        {
            if(i==0)
                if(j==0)
                _alpha(j,i)=1;//_emission(j,sequence[i])*_initial(0,j);
                 else
                 _alpha(j,i)=0;
            else
            {
                float s=0;
                for(int k=0;k<_nstates;k++)
                s=s+_transition(k,j)*_alpha(k,i-1);
                _alpha(j,i)=_emission(j,sequence[i-1])*s;
            }

        }
        float scale=0;
        for(int j=0;j<_nstates;j++)
        {
            scale=scale+_alpha(j,i);
        }
        scale=1.f/scale;
        if(i==0)
            _scale(0,i)=1;
        else
            _scale(0,i)=scale;

        for(int j=0;j<_nstates;j++)
        {
            _alpha(j,i)=scale*_alpha(j,i);

        }


        }


    }


    /**
     * @brief likelyhood : method to compute the log
     *                     likeyhood of observed sequence
     * @param sequence    :input observation sequence
     * @return
     */
    float likelyhood(vector<int> sequence)
    {
        float prob=0;
        //computing the probability of observed sequence
        //using forward algorithm
        forwardMatrix(sequence);
        backwardMatrix(sequence);


        for(int i=0;i<sequence.size()+1;i++)
        {
            //for(int j=0;j<_nstates;j++)
            {
                prob=prob+std::log(_scale(0,i));
            }



        }

        return -prob;
    }

};


/*class Gaussian
{
    Matrix _mu;
    Matrix _sigma;
    Matrix _invsigma;
    float _detsigma;
    float _detinvsigma;
    float scale;

//    enum class MTYPE:char{FULL=1,DIAGONAL=2,SHPERICAL=3};

    class gtype
    {
    public:
    enum type {TYPE_FULL, TYPE_DIAGONAL, TYPE_SPHERICAL};
    type t;
    };
    gtype t;


    void setParams(Mat mu,Mat sigma)
    {
        _mu.setData(mu);
        _sigma.setData(sigma);
        _invsigma=_sigma.inverse();
        _detsigma=_sigma.determinant();
        _detinvsigma=_invsigma.determinant();
        scale=1.f/pow(2*PI*_detsigma,0.5);
    }


    float Prob(Mat &x)
    {
        Matrix tmp;
        tmp.setData(x);

        float res=0;
        if(t.t==gtype::TYPE_FULL)
        {
        MatrixXf tmp1=(tmp._data-_mu._data).transpose();
        MatrixXf tmp2=tmp1*_invsigma._data*tmp1;
        res=tmp2(0,0);
        }
        else if(t.t==gtype::TYPE_SPHERICAL)
        {

        }
        else if(t.t==gtype::TYPE_DIAGONAL)
        {

        }


        res=exp(-0.5*res);
        if(res<0)
            res=0;
        if(res>1)
            res=1;
        if(isnan(res))
            res=0;

        return res;
    }

    float logProb(Mat &x)
    {
        ///
        Matrix tmp;
        tmp.setData(x);
        float res;

        if(t.t==gtype::TYPE_FULL)
        {
        MatrixXf tmp1=(tmp._data-_mu._data).transpose();
        MatrixXf tmp2=tmp1*_invsigma._data*tmp1;
        res=tmp2(0,0);
        }
        else if(t.t==gtype::TYPE_SPHERICAL)
        {

        }
        else if(t.t==gtype::TYPE_DIAGONAL)
        {

        }

        if(isnan(res))
            res=0;
        return res;
    }


};


/*
class GaussianMixture
{

    int nmix;
    enum class Type {FULL,DIAGONAL,TIED,SPHERICAL};
    Vector mu;
    Matrix sigma;

    float computeProbability(Vector x)
    {

    }


};


/*
class HMM
{
public:
    HMM();

    int nstates;
    int hmmLogprob()
    {
        /*
        nobs = numel(X);
        logp = zeros(nobs, 1);
        pi = model.pi;
        A = model.A;
        for i=1:nobs
            logB = mkSoftEvidence(model.emission, X{i});
            [logB, scale] = normalizeLogspace(logB');
            B    = exp(logB');
            logp(i) = hmmFilter(pi, A, B);
            logp(i) = logp(i) + sum(scale);
        end


    }

};
*/

#endif // HMM_H
