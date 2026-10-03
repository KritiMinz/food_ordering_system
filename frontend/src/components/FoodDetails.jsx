import { useEffect, useState } from "react"

function FoodDetails({
  food,
  onClose,
  onAddToCart,
  isFavorite,
  onToggleFavorite
}) {
  const [reviews, setReviews] = useState([])
  const [averageRating, setAverageRating] = useState(0)
  const [reviewCount, setReviewCount] = useState(0)

  const [rating, setRating] = useState(5)
  const [comment, setComment] = useState("")
  const [reviewMessage, setReviewMessage] = useState("")
  const [reviewError, setReviewError] = useState("")
  const [submittingReview, setSubmittingReview] = useState(false)

  const currentUser = JSON.parse(
    localStorage.getItem("currentUser") || "null"
  )

  const authToken = localStorage.getItem("authToken")

  // --------------------------------------------------
  // Load reviews
  // --------------------------------------------------

  useEffect(() => {
    if (!food) return

    fetch(`http://localhost:18080/api/foods/${food.id}/reviews`)
      .then((response) => response.json())
      .then((data) => {
        if (data.status === "success") {
          setReviews(data.reviews || [])
          setAverageRating(data.averageRating || 0)
          setReviewCount(data.reviewCount || 0)
        }
      })
      .catch(() => {
        setReviews([])
        setAverageRating(0)
        setReviewCount(0)
      })
  }, [food])

  if (!food) {
    return null
  }

  // --------------------------------------------------
  // Submit review
  // --------------------------------------------------

  const handleSubmitReview = async (event) => {
    event.preventDefault()

    setReviewMessage("")
    setReviewError("")

    if (!authToken || !currentUser) {
      setReviewError("Please login to submit a review.")
      return
    }

    if (!comment.trim()) {
      setReviewError("Please write a comment.")
      return
    }

    setSubmittingReview(true)

    try {
      const response = await fetch(
        "http://localhost:18080/api/reviews",
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${authToken}`
          },
          body: JSON.stringify({
            foodId: food.id,
            rating: Number(rating),
            comment: comment.trim()
          })
        }
      )

      const data = await response.json()

      if (!response.ok || data.status !== "success") {
        setReviewError(
          data.message || "Unable to submit review."
        )
        return
      }

      setReviewMessage("Review submitted successfully!")
      setComment("")
      setRating(5)

      // Reload reviews
      const reviewsResponse = await fetch(
        `http://localhost:18080/api/foods/${food.id}/reviews`
      )

      const reviewsData = await reviewsResponse.json()

      if (reviewsData.status === "success") {
        setReviews(reviewsData.reviews || [])
        setAverageRating(reviewsData.averageRating || 0)
        setReviewCount(reviewsData.reviewCount || 0)
      }
    } catch (error) {
      setReviewError("Unable to connect to the backend.")
    } finally {
      setSubmittingReview(false)
    }
  }

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4 py-6">

      <div className="bg-white rounded-2xl shadow-xl max-w-lg w-full max-h-[90vh] overflow-y-auto">

        {/* Food Image */}
        <div className="h-64 bg-orange-100 flex items-center justify-center text-8xl">
          {food.image}
        </div>

        <div className="p-6">

          {/* Header */}
          <div className="flex items-start justify-between">

            <div>
              <h2 className="text-3xl font-bold text-gray-800">
                {food.name}
              </h2>

              <p className="text-gray-500 mt-2">
                Category: {food.category}
              </p>
            </div>

            <button
              onClick={() => onToggleFavorite(food.id)}
              className="text-3xl"
            >
              {isFavorite ? "❤️" : "🤍"}
            </button>

          </div>

          {/* Description */}
          <p className="text-gray-600 mt-5">
            {food.description}
          </p>

          {/* Rating summary */}
          <div className="mt-5 bg-orange-50 rounded-xl p-4">

            <div className="flex items-center gap-3">

              <span className="text-3xl font-bold text-gray-800">
                {averageRating.toFixed(1)}
              </span>

              <div>
                <div className="text-xl">
                  {"★".repeat(Math.round(averageRating))}
                  <span className="text-gray-300">
                    {"★".repeat(
                      5 - Math.round(averageRating)
                    )}
                  </span>
                </div>

                <p className="text-sm text-gray-500">
                  {reviewCount}{" "}
                  {reviewCount === 1 ? "review" : "reviews"}
                </p>
              </div>

            </div>

          </div>

          {/* Price and cart */}
          <div className="flex items-center justify-between mt-6">

            <span className="text-2xl font-bold text-orange-600">
              ₹{food.price}
            </span>

            <button
              onClick={() => onAddToCart(food)}
              disabled={food.available === false}
              className={`px-6 py-3 rounded-lg font-semibold ${
                food.available === false
                  ? "bg-gray-400 text-white cursor-not-allowed"
                  : "bg-orange-600 text-white hover:bg-orange-700"
              }`}
            >
              {food.available === false
                ? "Currently Unavailable"
                : "Add to Cart"}
            </button>

          </div>

          {/* Reviews */}
          <div className="mt-8">

            <h3 className="text-xl font-bold text-gray-800 mb-4">
              Customer Reviews
            </h3>

            {reviews.length === 0 ? (
              <p className="text-gray-500">
                No reviews yet. Be the first to review this food!
              </p>
            ) : (
              <div className="space-y-3">

                {reviews.map((review) => (
                  <div
                    key={review.id}
                    className="border border-gray-200 rounded-xl p-4"
                  >

                    <div className="flex items-center justify-between">

                      <span className="font-semibold text-gray-700">
                        Customer #{review.userId}
                      </span>

                      <span className="text-yellow-500">
                        {"★".repeat(review.rating)}
                        <span className="text-gray-300">
                          {"★".repeat(5 - review.rating)}
                        </span>
                      </span>

                    </div>

                    <p className="text-gray-600 mt-2">
                      {review.comment}
                    </p>

                  </div>
                ))}

              </div>
            )}

          </div>

          {/* Write Review */}
          <div className="mt-8 border-t pt-6">

            <h3 className="text-xl font-bold text-gray-800 mb-4">
              Write a Review
            </h3>

            {!authToken ? (
              <p className="text-gray-500">
                Please login to write a review.
              </p>
            ) : (
              <form onSubmit={handleSubmitReview}>

                {/* Star selection */}
                <label className="block text-sm font-semibold text-gray-700 mb-2">
                  Your Rating
                </label>

                <div className="flex gap-2 mb-4">

                  {[1, 2, 3, 4, 5].map((star) => (
                    <button
                      key={star}
                      type="button"
                      onClick={() => setRating(star)}
                      className={`text-3xl ${
                        star <= rating
                          ? "text-yellow-400"
                          : "text-gray-300"
                      }`}
                    >
                      ★
                    </button>
                  ))}

                </div>

                {/* Comment */}
                <textarea
                  value={comment}
                  onChange={(event) =>
                    setComment(event.target.value)
                  }
                  placeholder="Write your review..."
                  rows="4"
                  className="w-full border border-gray-300 rounded-lg p-3
                             focus:outline-none focus:ring-2 focus:ring-orange-500"
                />

                {/* Success */}
                {reviewMessage && (
                  <p className="text-green-600 mt-3">
                    {reviewMessage}
                  </p>
                )}

                {/* Error */}
                {reviewError && (
                  <p className="text-red-600 mt-3">
                    {reviewError}
                  </p>
                )}

                <button
                  type="submit"
                  disabled={submittingReview}
                  className="w-full mt-4 bg-orange-600 text-white py-3
                             rounded-lg font-semibold hover:bg-orange-700
                             disabled:bg-gray-400"
                >
                  {submittingReview
                    ? "Submitting..."
                    : "Submit Review"}
                </button>

              </form>
            )}

          </div>

          {/* Close */}
          <button
            onClick={onClose}
            className="w-full mt-4 border border-gray-300 py-3 rounded-lg
                       text-gray-700 hover:bg-gray-100"
          >
            Close
          </button>

        </div>

      </div>

    </div>
  )
}

export default FoodDetails
